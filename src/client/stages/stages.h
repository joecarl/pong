#ifndef STAGES_H
#define STAGES_H

#include "../../ponggame.h"
#include <dp/client/stage.h>
#include <dp/client/ui/retrolines.h>
//#include <dp/client/mediatools.h>
#include <allegro5/allegro.h>


class MainMenuStage: public dp::client::Stage {

	int easteregg = 0;

	ALLEGRO_BITMAP* logo;

	dp::client::ui::RetroLines retro_logo;

public:

	MainMenuStage(dp::client::BaseClient* _engine);
	
	void on_enter_stage();

	void on_tick();

	void on_event(ALLEGRO_EVENT event);

	void draw();

};


//-----------------------------------------------------------------------------

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


//-----------------------------------------------------------------------------

class GameOverStage: public dp::client::Stage {

public:

	using Stage::Stage;

	void on_enter_stage();

	void on_event(ALLEGRO_EVENT event);

	void draw();

};


#endif
