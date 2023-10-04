#include "pongnetgroupshandler.h"
#include "pongclient.h"
#include "stages/stages.h" //game_handler
#include <iostream>

using std::cout, std::cerr, std::endl;


void OnlineGameController::push_event(boost::json::object &evt) {

	auto evt_type = evt["type"].as_string();

	if (evt_type == "sync") {
		
		cerr << "!! SYNCING: " << evt << endl;

		try {

			this->sync_game(evt["gamevars"].as_object());

		} catch (std::invalid_argument& e) {
			
			cerr << "SYNC ERROR:  " <<  e.what() << endl;

		}

	} else {
			
		//cout << "QUEUED: " << evt << endl;

		this->evt_queue.push(evt);

	}
	
}

void OnlineGameController::process_event(boost::json::object &evt) {

	//cout << "Processing evt" << evt << endl;

	auto evt_type = evt["type"].as_string();
		
	if (evt_type == "set_control_state") {
		
		int control = evt["control"].as_int64();
		bool new_state = evt["state"].as_bool();
		int player_key = evt["player_key"].as_int64();
		
		this->game->players[player_key]->controls[control] = new_state;

	} else if (evt_type == "set_paused_state") {

		bool new_state = evt["state"].as_bool();
		this->game->paused = new_state;

	} else {

		cerr << "Unknown event type: " << evt_type << endl;

	}

}


void sync_player(PlayerP* p, boost::json::object& vars) {

	p->x = vars["x"].as_int64();
	p->y = vars["y"].as_int64();
	//p->com_txt_y = vars["com_txt_y"].as_int64(); //this var is only processed in the client

	p->score = vars["score"].as_int64();
	p->medlen = vars["medlen"].as_int64();
	p->racha = vars["racha"].as_int64();

	unsigned int i;
	i = 0;
	for (auto &v: vars["bonus_timers"].as_array()) {
		p->bonus_timers[i++] = boost::json::value_to<unsigned int>(v);
	}

	i = 0;
	for (auto &v: vars["controls"].as_array()) {
		p->controls[i++] = v.as_bool();
	}
	
}

void sync_element(Element* e, boost::json::object& vars) {

	e->stat = vars["stat"].as_bool();
	e->x = vars["x"].as_double();
	e->y = vars["y"].as_double();
	e->radius = vars["radius"].as_double();
	e->vx = vars["vx"].as_double();
	e->vy = vars["vy"].as_double();

}

void sync_wall(Wall* w, boost::json::object& vars) {
	
	sync_element((Element*) w, vars);
	
}

void sync_ball(Ball* b, boost::json::object& vars) {
	
	sync_element((Element*) b, vars);
	
}

void sync_bonus(Bonus* b, boost::json::object& vars) {

	sync_element((Element*) b, vars);
	b->cooldown = vars["cooldown"].as_int64();
	
}


void OnlineGameController::sync_game(boost::json::object& vars) {

	this->game->tick = boost::json::value_to<unsigned int>(vars["tick"]);
	this->game->warmup = boost::json::value_to<uint16_t>(vars["warmup"]);
	this->game->paused = vars["paused"].as_bool();
	this->game->rnd.index = vars["rnd_index"].as_int64();

	cout << "sync players ..." << endl;
	sync_player(this->game->players[0], vars["p0vars"].as_object());
	sync_player(this->game->players[1], vars["p1vars"].as_object());
	
	cout << "sync bonus ..." << endl;
	for (uint8_t i = 0; i < BONUS_MAX; i++) {
		sync_bonus(this->game->bonus[i], vars["bonus"].as_array()[i].as_object());
	}

	cout << "sync walls ..." << endl;
	for (uint8_t i = 0; i < 4; i++) {
		sync_wall(this->game->walls[i], vars["walls"].as_array()[i].as_object());
	}

	cout << "sync ball ..." << endl;
	sync_ball(this->game->ball, vars["ballvars"].as_object());

}



void OnlineGameController::setup(PongGame *game) {
	
	this->game = game;
	
	//vaciamos la cola de eventos
	std::queue<boost::json::object> empty;
	std::swap(this->evt_queue, empty);

}


void OnlineGameController::on_tick() {

	while (this->evt_queue.size() > 0) {

		auto evt = this->evt_queue.front();
		auto evt_tick = (unsigned int) evt["tick"].as_int64();

		if (evt_tick == this->game->tick) {

			this->process_event(evt);

			this->evt_queue.pop();

		} else if (evt_tick < this->game->tick) {

			cerr << "DESYNC! evt_tick: " << evt_tick << " | game_tick: " << this->game->tick << endl;

			this->evt_queue.pop();
		
			throw std::runtime_error("Evento perdido");

		} else {
			
			break;

		}

	}
}



PongNetGroupsHandler::PongNetGroupsHandler(PongClient* client) : 
	dp::client::NetGroupsHandler(client)
{
	this->nelh->add_event_listener("net/set_client_id", [this] (boost::json::object& data) {
		auto cl = this->get_client();
		cl->get_io_client().send_event("client/login", { {"cfg", cl->get_cfg()} });
	});
}


void PongNetGroupsHandler::create_group(dp::client::Connection* net, std::string id, std::string owner_id, boost::json::array& members) {
	
	NetGroupsHandler::create_group(net, id, owner_id, members);
	
	auto local_id = net->get_local_id();
	auto group_nelh = this->group->get_nelh();
	group_nelh->add_event_listener("group/game_start", [this, local_id] (boost::json::object& data) {

		PongClient* cl = static_cast<PongClient*>(this->get_client());
		auto& game_handler = cl->get_game_handler();

		game_handler.make_new_pong_game((int_fast32_t) data["seed"].as_int64());
		controller.setup(game_handler.pong_game);

		auto players_order = data["players_order"].as_array(); //quiza podria omitirse y utilizar el mismo orden que haya en el grupo, pero no me fio de que se trafuque
		uint8_t i = 0;
		for (auto& ord: players_order) {
			std::string client_id = ord.as_string().c_str();
			auto info = this->group->get_member_info(client_id);
			if (client_id == local_id) {
				game_handler.local_player_idx = i;
			}
			//cout << "curioso " << player_cfg << endl;
			game_handler.set_player_name(i, info->name);
			i++;
		}
		
		PongClient* client = static_cast<PongClient*>(this->get_client());
		client->set_stage(GAME);
	
	});

	// TODO: ver bien como gestionar esto: (quiza la clase controller puede ir a pronggroup y game_handler a pongclient)
	group_nelh->add_event_listener("game/event", [this] (boost::json::object& data) {

		controller.push_event(data);
		
	});
	
}