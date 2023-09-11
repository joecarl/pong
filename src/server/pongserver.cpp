#include "pongserver.h"
#include <dp/serendipia.h>
#include <boost/json.hpp>

boost::json::object export_player(PlayerP* p) {

	boost::json::object o;

	o["x"] = p->x;
	o["y"] = p->y;
	o["com_txt_y"] = p->com_txt_y;
	o["score"] = p->score;
	o["medlen"] = p->medlen;
	o["racha"] = p->racha;

	boost::json::array arr1;
	for (auto &v: p->bonus_timers) {
		arr1.push_back(v);
	}
	o["bonus_timers"] = arr1;

	boost::json::array arr2;
	for (auto &v: p->controls) {
		arr2.push_back(v);
	}
	o["controls"] = arr2;
	
	return o;

}

boost::json::object export_element(Element* e) {

	boost::json::object o;

	o = {
		{"stat", e->stat},
		{"x", e->x},
		{"y", e->y},
		{"radius", e->radius},
		{"vx", e->vx},
		{"vy", e->vy},
	};

	return o;

}

boost::json::object export_ball(Ball* b) {
	
	return export_element((Element*) b);
	
}

boost::json::object export_wall(Wall* w) {
	
	// no exporto owner_idx porque es invariable
	return export_element((Element*) w);
	
}

boost::json::object export_bonus(Bonus* b) {

	boost::json::object o = export_element((Element*) b);
	o["cooldown"] = b->cooldown;
	
	return o;

}


boost::json::object _export_game(PongGame* g) {

	boost::json::object o;

	boost::json::array bonus;
	for (uint8_t i = 0; i < BONUS_MAX; i++) {
		bonus.push_back(export_bonus(g->bonus[i]));
	}

	boost::json::array walls;
	for (uint8_t i = 0; i < 4; i++) {
		walls.push_back(export_wall(g->walls[i]));
	}

	o = {
		{"tick", g->tick},
		{"warmup", g->warmup},
		{"paused", g->paused},
		{"rnd_index", g->rnd.index},
		{"p0vars", export_player(g->players[0])},
		{"p1vars", export_player(g->players[1])},
		{"bonus", bonus},
		{"walls", walls},
		{"ballvars", export_ball(g->ball)}
	};

	return o;

}


dp::BaseGame* PongServer::create_game() {

	PongGame* game = new PongGame();
	game->control_mode = CONTROLMODE_TWO_PLAYERS;
	game->restart();
	game->iniciar_punto(1);

	return static_cast<dp::BaseGame*>(game);

}


boost::json::object PongServer::export_game(dp::BaseGame* game) {

	return _export_game(static_cast<PongGame*>(game));
	
}
