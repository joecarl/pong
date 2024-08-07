#ifndef GAMESTAGE_H
#define GAMESTAGE_H

#include "../../ponggame.h"
#include <dp/client/stage.h>
#include <allegro5/allegro.h>
#include <string>

class Tracer {

	dp::client::BaseClient* engine;

	ALLEGRO_BITMAP* bonus_sprites[BONUS_MAX];

public:

	Tracer(dp::client::BaseClient* _engine);

	ALLEGRO_BITMAP* get_sprite_for_bonus_type(BonusType bonus_type);

	void draw_ball(Ball *b, float scale);

	void draw_bonus(Bonus *b, float scale);

	void draw_player(PlayerP *pl, int scale);

	void draw_wall(Wall *w, float scale);

};


//-----------------------------------------------------------------------------

class GameStage: public dp::client::Stage {

	void draw_court();

	void draw_scores();

	void trigger_desync();

	Tracer *tracer;

public:

	int delayer;

	GameStage(dp::client::BaseClient* _engine);

	void on_enter_stage();

	void on_event(ALLEGRO_EVENT evt);

	void on_tick();

	void draw();

	void process_message(std::string &msg);

};

#endif