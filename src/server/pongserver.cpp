#include "pongserver.h"
#include <dp/serendipia.h>
#include <dp/object.h>
#include <boost/json.hpp>

using dp::Object;

Object export_player(PlayerP* p) {

	boost::json::array arr1;
	for (auto &v: p->bonus_timers) {
		arr1.push_back(v);
	}

	boost::json::array arr2;
	for (auto &v: p->controls) {
		arr2.push_back(v);
	}
	
	return {
		{"x", p->x},
		{"y", p->y},
		{"com_txt_y", p->com_txt_y}, 
		{"score", p->score}, 
		{"medlen", p->medlen}, 
		{"racha", p->racha}, 
		{"bonus_timers", arr1}, 
		{"controls", arr2},
	};

}

Object export_element(Element* e) {

	return {
		{"stat", e->stat},
		{"x", e->x},
		{"y", e->y},
		{"radius", e->radius},
		{"vx", e->vx},
		{"vy", e->vy},
	};

}

Object export_ball(Ball* b) {
	
	return export_element((Element*) b);
	
}

Object export_wall(Wall* w) {
	
	// no exporto owner_idx porque es invariable
	return export_element((Element*) w);
	
}

Object export_bonus(Bonus* b) {

	Object o = export_element((Element*) b);
	o.set("cooldown", b->cooldown);
	
	return o;

}


Object _export_game(PongGame* g) {

	boost::json::array bonus;
	for (uint8_t i = 0; i < BONUS_MAX; i++) {
		bonus.push_back(export_bonus(g->bonus[i]).json());
	}

	boost::json::array walls;
	for (uint8_t i = 0; i < 4; i++) {
		walls.push_back(export_wall(g->walls[i]).json());
	}

	return {
		{"tick", g->tick},
		{"warmup", g->warmup},
		{"paused", g->paused},
		{"rnd_index", g->rnd.index},
		{"p0vars", export_player(g->players[0]).json()},
		{"p1vars", export_player(g->players[1]).json()},
		{"bonus", bonus},
		{"walls", walls},
		{"ballvars", export_ball(g->ball).json()}
	};

}


dp::BaseGame* PongServer::create_game() {

	PongGame* game = new PongGame();
	game->control_mode = CONTROLMODE_TWO_PLAYERS;
	game->restart();
	game->iniciar_punto(1);

	return static_cast<dp::BaseGame*>(game);

}


Object PongServer::export_game(dp::BaseGame* game) {

	return _export_game(static_cast<PongGame*>(game));
	
}
