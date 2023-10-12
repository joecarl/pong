#include "pongnetgroupshandler.h"
#include "pongclient.h"
#include "stages/stages.h" //game_handler
#include <iostream>

using std::cout, std::cerr, std::endl;
using dp::Object;

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

void sync_game(PongGame* game, const Object& vars) {

	game->tick = vars["tick"];
	game->warmup = vars["warmup"];
	game->paused = vars["paused"];
	game->rnd.index = vars["rnd_index"];

	//cout << "sync players ..." << endl;
	sync_player(game->players[0], vars["p0vars"]);
	sync_player(game->players[1], vars["p1vars"]);
	
	//cout << "sync bonus ..." << endl;
	boost::json::array bonus = vars["bonus"];
	for (uint8_t i = 0; i < BONUS_MAX; i++) {
		sync_bonus(game->bonus[i], Object(bonus[i]));
	}

	//cout << "sync walls ..." << endl;
	boost::json::array walls = vars["walls"];
	for (uint8_t i = 0; i < 4; i++) {
		sync_wall(game->walls[i], Object(walls[i]));
	}

	//cout << "sync ball ..." << endl;
	sync_ball(game->ball, vars["ballvars"]);

}


PongNetGroupsHandler::PongNetGroupsHandler(PongClient* client) : 
	dp::client::NetGroupsHandler(client)
{
	this->controller.sync_game = [] (dp::BaseGame* g, const dp::Object& v) { 
		sync_game(static_cast<PongGame*>(g), v); 
	};

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
		
		uint8_t i = 0;
		for (auto& info: this->group->get_members()) {

			if (info.client_id == local_id) {
				game_handler.local_player_idx = i;
			}
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