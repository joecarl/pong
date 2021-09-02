#ifndef STAGES_H
#define STAGES_H

#include <allegro5/allegro.h>
#include "hengine.h"
#include "ioclient.h"
#include "utils.h"


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


//-----------------------------------------------------------------------------

class ConnStage: public Stage{

	JC_TEXTINPUT* input;

	std::string server;

	bool start_connection = 0;

	ioClient connection;
	
public:

	ConnStage(HGameEngine* _engine);

	void onEnterStage();

	void onEvent(ALLEGRO_EVENT event);

	void draw();

};

#endif
