#ifndef HENGINE_H
#define HENGINE_H

#include "ioclient.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <boost/json.hpp>

#define TICKS_PER_SECOND 60.0

class HGameEngine;

enum {
	MENU = 0,
	GAME,
	OVER,
	CONN,
	LOBBY,
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

	IoClient connection;
	
	ALLEGRO_FONT* font;

	boost::json::object cfg;

	float fps = 0;
	
	float scale = 1.0;

	unsigned int activeStageID = 0;

	bool finish = false;

	Stage* stages[MAX_SCREENS];

	bool keys[300];

	int resX = 320/*DEF_W*/, resY = 200/*DEF_H*/;

	HGameEngine();

	void calcFPS();

	void setStage(unsigned int stageID);

	void runTick();

	void onEvent(ALLEGRO_EVENT event);

	void draw();

	void run();

};

#endif
