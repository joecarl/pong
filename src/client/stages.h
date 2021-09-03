#ifndef STAGES_H
#define STAGES_H

#include "hengine.h"
#include "mediatools.h"
#include "../classes.h"
#include <allegro5/allegro.h>

#ifdef __APPLE__
#define LONG_DIR "../Resources/long.bmp"
#define BALL_DIR "../Resources/ball.bmp"
#define LOGO_DIR "../Resources/pong.bmp"
#define FONT_DIR "../Resources/font.ttf"

#elif defined _WIN32 || defined _WIN64
#define LONG_DIR "resources/long.bmp"
#define BALL_DIR "resources/ball.bmp"
#define LOGO_DIR "resources/pong.bmp"
#define FONT_DIR "resources/font.ttf"
#endif


enum{
	PLAYMODE_NONE = 0,
	PLAYMODE_SINGLE_PLAYER,
	PLAYMODE_TWO_PLAYERS,
	PLAYMODE_TRAINING,
	PLAYMODE_ONLINE
};


class GameHandler{

public:

	int playMode = PLAYMODE_NONE;

	PongGame* pongGame = nullptr;

	~GameHandler();

	void makeNewPongGame(int_fast32_t seed);
	
	void cleanup();

};

extern GameHandler gameHandler;


//----------------------------------------------------------------------------

class MainMenuStage: public Stage{

	int easteregg = 0;

	ALLEGRO_BITMAP* logo;

public:

	MainMenuStage(HGameEngine* _engine);

	void onTick();

	void onEvent(ALLEGRO_EVENT event);

	void draw();

};


//-----------------------------------------------------------------------------

class Tracer{

	HGameEngine* engine;

	ALLEGRO_BITMAP *bonus_ball_spr, *bonus_long_spr;

public:

	Tracer(HGameEngine* _engine);

	ALLEGRO_BITMAP* getSpriteForBonusType(int bonus_type);

	void drawBall(Ball *b, float scale);

	void drawBonus(Bonus * b, float scale);

	void drawPlayer(PlayerP *pl, int scale);

};


//-----------------------------------------------------------------------------

class GameStage: public Stage{

	void drawCourt();

	void drawScores();

	Tracer *tracer;

public:

	int delayer;

	GameStage(HGameEngine* _engine);

	void onEnterStage();

	void onEvent(ALLEGRO_EVENT evt);

	void onTick();

	void draw();

	void processMessage(std::string &msg);

};


//-----------------------------------------------------------------------------

class GameOverStage: public Stage{

public:

	using Stage::Stage;

	void onEnterStage();

	void onEvent(ALLEGRO_EVENT event);

	void draw();

};


#endif
