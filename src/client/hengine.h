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
	TUTO,
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

	/**
	 * Main buffer where all non-UI components will render
	 */
	ALLEGRO_BITMAP *buffer;

	/**
	 * Secondary buffer mainly used to render UI components
	 */
	ALLEGRO_BITMAP *sec_buffer;

	ALLEGRO_EVENT_QUEUE *event_queue;
	/**
	 * How much the buffer should be scaled
	 */
	int scale_w, scale_h, scale_x, scale_y;

	int window_width, window_height;

	float scaled;

	void cleanup();

public:

	AllegroHandler(HGameEngine *game_engine);

	~AllegroHandler();

	void initialize_resources();

	void create_components();

	Point get_mapped_coordinates(int real_x, int real_y);

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

	ALLEGRO_EVENT_QUEUE* get_event_queue() { return this->event_queue; }

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

	std::string custom_cfg_filepath;

	AllegroHandler allegro_hnd;

	TouchKeys touch_keys;

	TouchKeys kb_touch_keys;

	IoClient connection;

	ALLEGRO_FONT* font;

	boost::json::object cfg;

	float fps = 0;

	float scale = 1.0;

	unsigned int active_stage_id = 0;

	Stage* stages[MAX_SCREENS];

	bool keys[ALLEGRO_KEY_MAX];

	uint16_t res_x = 320/*DEF_W*/, res_y = 200/*DEF_H*/;

	void calc_fps();

	void run_tick();

	void on_event(ALLEGRO_EVENT event);

	void draw();

public:

	std::string debug_txt = "";

	bool finish = false;

	HGameEngine();

	void set_stage(unsigned int stage_id);

	void run();

	void set_cfg_param(const std::string& key, const boost::json::value& val);

	TouchKeys& get_touch_keys() { return this->touch_keys; }

	IoClient& get_io_client() { return this->connection; }

	boost::json::object& get_cfg() { return this->cfg; }

	AllegroHandler& get_allegro_hnd() { return this->allegro_hnd; }

	float get_scale() { return this->scale; }

	float get_fps() { return this->fps; }

	uint16_t get_res_x() { return this->res_x; }

	uint16_t get_res_y() { return this->res_y; }

	ALLEGRO_FONT* get_font() { return this->font; }

	bool get_key(uint16_t kcode) { return kcode < ALLEGRO_KEY_MAX ? this->keys[kcode]: false; }

};

#endif
