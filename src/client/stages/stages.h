#ifndef STAGES_H
#define STAGES_H

#include "../hengine.h"
#include "../mediatools.h"
#include "../textinput.h"
#include "../../ponggame.h"
#include <allegro5/allegro.h>


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

typedef struct {
	uint8_t x;
	uint8_t width;
} Dash;

typedef std::vector<Dash> RetroLine;


class RetroLines {

	std::vector<RetroLine> lines;

	uint8_t width = 0;

	uint8_t mult_x = 6;

	uint8_t mult_y = 4;

	float time = 0;

	void calc_width();

public:

	RetroLines(std::vector<RetroLine>&& _lines);

	RetroLines(std::vector<std::string>&& str_lines);

	void draw(float ox, float oy);

	uint8_t get_width() { return this->width; }

};

class MainMenuStage: public Stage {

	int easteregg = 0;

	ALLEGRO_BITMAP* logo;

	RetroLines retro_logo;

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

	ALLEGRO_BITMAP* bonus_sprites[BONUS_MAX];

public:

	Tracer(HGameEngine* _engine);

	ALLEGRO_BITMAP* get_sprite_for_bonus_type(BonusType bonus_type);

	void draw_ball(Ball *b, float scale);

	void draw_bonus(Bonus *b, float scale);

	void draw_player(PlayerP *pl, int scale);

};


//-----------------------------------------------------------------------------

class GameStage: public Stage {

	void draw_court();

	void draw_scores();

	void trigger_desync();

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


//-----------------------------------------------------------------------------

class ConfigStage: public Stage {

	TextInput* input;

public:

	ConfigStage(HGameEngine* _engine);

	void on_enter_stage();

	void on_event(ALLEGRO_EVENT event);

	void draw();

};



#endif
