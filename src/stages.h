#ifndef STAGES_H
#define STAGES_H

#include <allegro5/allegro.h>
#include "hengine.h"
#include "ioclient.h"
#include "utils.h"


class MainMenuStage: public Stage{

	int easteregg = 0;

	ALLEGRO_BITMAP* logo;

public:

	MainMenuStage(HGameEngine* _engine);

	void onTick();

	void onEvent(ALLEGRO_EVENT event);

	void draw();

};

class GameStage: public Stage{

	void drawCourt();

	void drawScores();

public:

	using Stage::Stage;

	int delayer;

	void onEnterStage();

	void onEvent(ALLEGRO_EVENT evt);

	void draw();

};

class GameOverStage: public Stage{

public:

	using Stage::Stage;

	void onEnterStage();

	void onEvent(ALLEGRO_EVENT event);

	void draw();

};

class ConnStage: public Stage{

	JC_TEXTINPUT* input;

	string server;

	bool start_connection = 0;

	ioClient connection;
	
public:

	ConnStage(HGameEngine* _engine);

	void onEnterStage();

	void onEvent(ALLEGRO_EVENT event);

	void draw();

};

#endif
