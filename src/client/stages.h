#ifndef STAGES_H
#define STAGES_H

#include "hengine.h"
#include "mediatools.h"
#include "../ponggame.h"
#include <allegro5/allegro.h>

#ifdef __APPLE__
#define LONG_DIR "../Resources/long.bmp"
#define BALL_DIR "../Resources/ball.bmp"
#define LOGO_DIR "../Resources/pong.bmp"
#define FONT_DIR "../Resources/font.ttf"

#else //if defined _WIN32 || defined _WIN64
#define LONG_DIR "resources/long.bmp"
#define BALL_DIR "resources/ball.bmp"
#define LOGO_DIR "resources/pong.bmp"
#define FONT_DIR "resources/font.ttf"
#endif


enum {
	PLAYMODE_NONE = 0,
	PLAYMODE_LOCAL,
	PLAYMODE_ONLINE
};


class GameHandler {

	int control_mode = CONTROLMODE_NONE;

public:

	int play_mode = PLAYMODE_NONE;

	PongGame* pong_game = nullptr;

	~GameHandler();

	void setup(int play_mode, int control_mode);

	void make_new_pong_game(int_fast32_t seed);
	
	int get_control(int kCode, int playerID);
	
	void cleanup();

};

extern GameHandler game_handler;


//----------------------------------------------------------------------------

class MainMenuStage: public Stage {

	int easteregg = 0;

	ALLEGRO_BITMAP* logo;

public:

	MainMenuStage(HGameEngine* _engine);
	
	void on_enter_stage();

	void on_tick();

	void on_event(ALLEGRO_EVENT event);

	void draw();

};


//-----------------------------------------------------------------------------

class Tracer {

	HGameEngine* engine;

	ALLEGRO_BITMAP *bonus_ball_spr, *bonus_long_spr;

public:

	Tracer(HGameEngine* _engine);

	ALLEGRO_BITMAP* get_sprite_for_bonus_type(int bonus_type);

	void draw_ball(Ball *b, float scale);

	void draw_bonus(Bonus * b, float scale);

	void draw_player(PlayerP *pl, int scale);

};


//-----------------------------------------------------------------------------

class GameStage: public Stage {

	void draw_court();

	void draw_scores();

	Tracer *tracer;

public:

	int delayer;

	GameStage(HGameEngine* _engine);

	void on_enter_stage();

	void on_event(ALLEGRO_EVENT evt);

	void on_tick();

	void draw();

	void process_message(std::string &msg);

};


//-----------------------------------------------------------------------------

class GameOverStage: public Stage {

public:

	using Stage::Stage;

	void on_enter_stage();

	void on_event(ALLEGRO_EVENT event);

	void draw();

};


#endif
