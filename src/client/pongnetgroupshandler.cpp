#include "pongnetgroupshandler.h"
#include "pongclient.h"
#include "stages/stages.h" //game_handler
#include <iostream>

using std::cout, std::cerr, std::endl;
using dp::Object;


void OnlineGameController::push_event(const Object &evt) {

	auto evt_type = evt.sget<std::string>("type");

	if (evt_type == "sync") {
		
		cerr << "!! SYNCING: " << evt << endl;

		try {

			this->sync_game(evt["gamevars"]);

		} catch (std::invalid_argument& e) {
			
			cerr << "SYNC ERROR:  " <<  e.what() << endl;

		}

	} else {
			
		//cout << "QUEUED: " << evt << endl;

		this->evt_queue.push(evt);

	}
	
}

void OnlineGameController::process_event(const Object &evt) {

	//cout << "Processing evt" << evt << endl;

	auto evt_type = evt.sget<std::string>("type");
		
	if (evt_type == "set_control_state") {
		int control = evt["control"]; 
		bool new_state = evt["state"];
		int player_key = evt["player_key"];
		this->game->players[player_key]->controls[control] = new_state;

	} else if (evt_type == "set_paused_state") {

		bool new_state = evt["state"];
		this->game->paused = new_state;

	} else {

		cerr << "Unknown event type: " << evt_type << endl;

	}

}


void sync_player(PlayerP* p, const Object& vars) {

	p->x = vars["x"];
	p->y = vars["y"];
	//p->com_txt_y = vars["com_txt_y"].as_int64(); //this var is only processed in the client

	p->score = vars["score"];
	p->medlen = vars["medlen"];
	p->racha = vars["racha"];

	unsigned int i;
	i = 0;
	boost::json::array bonus_timers = vars["bonus_timers"];
	for (auto &v: bonus_timers) {
		p->bonus_timers[i++] = boost::json::value_to<unsigned int>(v);
	}

	i = 0;
	boost::json::array controls = vars["controls"];
	for (auto &v: controls) {
		p->controls[i++] = v.as_bool();
	}
	
}

void sync_element(Element* e, const Object& vars) {

	e->stat = vars["stat"];
	e->x = vars["x"];
	e->y = vars["y"];
	e->radius = vars["radius"];
	e->vx = vars["vx"];
	e->vy = vars["vy"];

}

void sync_wall(Wall* w, const Object& vars) {
	
	sync_element((Element*) w, vars);
	
}

void sync_ball(Ball* b, const Object& vars) {
	
	sync_element((Element*) b, vars);
	
}

void sync_bonus(Bonus* b, const Object& vars) {

	sync_element((Element*) b, vars);
	b->cooldown = vars["cooldown"];
	
}


void OnlineGameController::sync_game(const Object& vars) {

	this->game->tick = vars["tick"];
	this->game->warmup = vars["warmup"];
	this->game->paused = vars["paused"];
	this->game->rnd.index = vars["rnd_index"];

	cout << "sync players ..." << endl;
	sync_player(this->game->players[0], vars["p0vars"]);
	sync_player(this->game->players[1], vars["p1vars"]);
	
	cout << "sync bonus ..." << endl;
	boost::json::array bonus = vars["bonus"];
	for (uint8_t i = 0; i < BONUS_MAX; i++) {
		sync_bonus(this->game->bonus[i], Object(bonus[i]));
	}

	cout << "sync walls ..." << endl;
	boost::json::array walls = vars["walls"];
	for (uint8_t i = 0; i < 4; i++) {
		sync_wall(this->game->walls[i], Object(walls[i]));
	}

	cout << "sync ball ..." << endl;
	sync_ball(this->game->ball, vars["ballvars"]);

}



void OnlineGameController::setup(PongGame *game) {
	
	this->game = game;
	
	//vaciamos la cola de eventos
	std::queue<dp::Object> empty;
	std::swap(this->evt_queue, empty);

}


void OnlineGameController::on_tick() {

	while (this->evt_queue.size() > 0) {

		auto& evt = this->evt_queue.front();
		uint64_t evt_tick = evt["tick"];

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
	this->nelh->add_event_listener("net/set_client_id", [this] (const dp::Object& data) {
		auto cl = this->get_client();
		cl->get_io_client().send_event("client/login", { {"cfg", cl->get_cfg().json()} });
	});
}


void PongNetGroupsHandler::create_group(dp::client::Connection* net, std::string id, std::string owner_id, boost::json::array& members) {
	
	NetGroupsHandler::create_group(net, id, owner_id, members);
	
	auto local_id = net->get_local_id();
	auto group_nelh = this->group->get_nelh();
	group_nelh->add_event_listener("group/game_start", [this, local_id] (const dp::Object& data) {

		PongClient* cl = static_cast<PongClient*>(this->get_client());
		auto& game_handler = cl->get_game_handler();

		game_handler.make_new_pong_game((int_fast32_t) data.get<int64_t>("seed"));
		controller.setup(game_handler.pong_game);
		/*
		auto players_order = data.get<boost::json::array>("players_order"); //quiza podria omitirse y utilizar el mismo orden que haya en el grupo, pero no me fio de que se trafuque
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
		*/
		uint8_t i = 0;
		for (auto& info: this->group->get_members()) {

			if (info.client_id == local_id) {
				game_handler.local_player_idx = i;
			}
			//cout << "curioso " << player_cfg << endl;
			game_handler.set_player_name(i, info.name);
			i++;
		}
		
		PongClient* client = static_cast<PongClient*>(this->get_client());
		client->set_stage(GAME);
	
	});

	group_nelh->add_event_listener("game/event", [this] (const dp::Object& data) {

		controller.push_event(data);
		
	});
	
}