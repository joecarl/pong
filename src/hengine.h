#ifndef HENGINE_H
#define HENGINE_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

#include "classes.h"//move pongGame to specific stage!

#define TICKS_PER_SECOND 60.0

using namespace std;

class HGameEngine;

enum {
	MENU = 0,
	GAME = 1,
	OVER = 2,
	CONN = 3,
	MAX_SCREENS
};


class AllegroHandler{
	
	HGameEngine *engine;

	ALLEGRO_TIMER* timer;

	ALLEGRO_DISPLAY *display = NULL;

	int screenWidth, screenHeight;

	ALLEGRO_BITMAP *buffer;
	// how much the buffer should be scaled
	int scaleW, scaleH, scaleX, scaleY;

public:
	
	ALLEGRO_EVENT_QUEUE *event_queue;

	AllegroHandler(HGameEngine *gameEngine);

	void initializeResources();

	void createComponents();

	void startDrawing();

	void finishDrawing();

	void cleanup();

};

class Stage{
	
public:

	HGameEngine* engine;
	
	Stage(HGameEngine* _engine);

	virtual void onEvent(ALLEGRO_EVENT event);

	virtual void onTick();

	virtual void draw();

	virtual void onEnterStage();

};

class HGameEngine{

	double old_time = 0;
	
	unsigned int frames_done = 0;

	bool mustRunOnEnterStage = false;

	AllegroHandler* allegroHnd;

public:

	ALLEGRO_FONT* font;

	float fps = 0;
	
	float scale = 1.0;

	unsigned int activeStageID = 0;

	bool finish = false;

	Stage* stages[MAX_SCREENS];

	PongGame* pongGame;

	bool keys[300];

	int resX = DEF_W, resY = DEF_H;

	HGameEngine();

	void calcFPS();

	void setStage(unsigned int stageID);

	void runTick();

	void onEvent(ALLEGRO_EVENT event);

	void draw();

	void run();

};

#endif
