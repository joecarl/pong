#ifndef HENGINE_H
#define HENGINE_H

#include "ioclient.h"
#include "touchkeys.h"

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

struct Point {
	int x, y;
};

class AllegroHandler {
	
	HGameEngine *engine;

	ALLEGRO_TIMER* timer;

	ALLEGRO_DISPLAY *display = NULL;

	int screen_width, screen_height;

	ALLEGRO_BITMAP *buffer;
	ALLEGRO_BITMAP *sec_buffer;
	// how much the buffer should be scaled
	int scale_w, scale_h, scale_x, scale_y;

	int window_width, window_height;

	float scaled;

public:
	
	ALLEGRO_EVENT_QUEUE *event_queue;

	AllegroHandler(HGameEngine *gameEngine);

	void initialize_resources();

	void create_components();

	Point get_mapped_coordinates(int realX, int realY);

	void fit_display();

	int get_window_width();

	int get_window_height();

	void start_drawing();

	void prepare_main_surface();

	void draw_main_surface();

	void prepare_sec_surface();

	void draw_sec_surface();

	void finish_drawing();

	float get_scaled();

	void cleanup();

};

class Stage {
	
public:

	HGameEngine* engine;
	
	Stage(HGameEngine* _engine);

	virtual void on_event(ALLEGRO_EVENT event);

	virtual void on_tick();

	virtual void draw();

	virtual void on_enter_stage();

};

class HGameEngine {

	double old_time = 0;
	
	unsigned int frames_done = 0;

	bool must_run_on_enter_stage = false;

public:

	std::string debug_txt = "";

	AllegroHandler* allegro_hnd;

	TouchKeys touch_keys;

	IoClient connection;
	
	ALLEGRO_FONT* font;

	boost::json::object cfg;

	float fps = 0;
	
	float scale = 1.0;

	unsigned int active_stage_id = 0;

	bool finish = false;

	Stage* stages[MAX_SCREENS];

	bool keys[300];

	int res_x = 320/*DEF_W*/, res_y = 200/*DEF_H*/;

	HGameEngine();

	void calcFPS();

	void set_stage(unsigned int stage_id);

	void run_tick();

	void on_event(ALLEGRO_EVENT event);

	void draw();

	void run();

};

#endif
