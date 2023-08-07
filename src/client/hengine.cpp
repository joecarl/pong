#include "hengine.h"
#include "stages.h"
#include "onlinestages.h"
#include "../utils.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#ifdef ALLEGRO_ANDROID
#include <allegro5/allegro_android.h> /* al_android_set_apk_file_interface */
#endif

#include <iostream>

using namespace std;

//-----------------------------------------------------------------------------
//---------------------------- [AllegroHandler] -------------------------------

AllegroHandler::AllegroHandler(HGameEngine *gameEngine) {
	
	this->engine = gameEngine;

}

void AllegroHandler::initialize_resources() {

	cout << "Initializing addons..." << endl;
	al_init_image_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_primitives_addon();
	
	cout << "Initializing allegro..." << endl;
	if (!al_init()) {
		throw std::runtime_error("failed to initialize allegro!");
	}
	
	if (!al_install_audio()) {
		throw std::runtime_error("could not init sound!");
	}
	if (!al_reserve_samples(1)) {
		throw std::runtime_error("could not reserve samples!"); 
	}
	
	cout << "Installing keyboard..." << endl;
	if (!al_install_keyboard()) {
		throw std::runtime_error("could not init keyboard!");
	}
	
	#ifdef ALLEGRO_ANDROID
	cout << "Installing touch input..." << endl;
	if (!al_install_touch_input()) {
		throw std::runtime_error("could not init touch input!");
	}
	
	al_android_set_apk_file_interface();
	#endif

	cout << "Allegro initialized" << endl;

}

void AllegroHandler::create_components() {

	bool windowed = this->engine->cfg["windowed"].as_bool();
	
	if (!windowed) {
		al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
	}

	//al_set_new_display_option(ALLEGRO_SUPPORTED_ORIENTATIONS, ALLEGRO_DISPLAY_ORIENTATION_LANDSCAPE, ALLEGRO_SUGGEST);
	al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);

	//ALLEGRO_DISPLAY_MODE disp_data;
	/*
	for (int i = 0; i < al_get_num_display_modes(); i++) {
		al_get_display_mode(i, &disp_data);
		cout << "Resolution: " << disp_data.width << "x" << disp_data.height << endl;
	}
	*/	
	//al_get_display_mode(al_get_num_display_modes() - 1, &disp_data);

	this->screen_width = this->engine->scale * this->engine->res_x;
	this->screen_height = this->engine->scale * this->engine->res_y;

	display = al_create_display(640, 400);//disp_data.width, disp_data.height);
	if (!display) {
		throw std::runtime_error("failed to create display!");
	}

	this->buffer = al_create_bitmap(screen_width, screen_height);
	this->sec_buffer = nullptr;

	this->fit_display();

	if (!windowed) {
		al_hide_mouse_cursor(display);
	}
	
	cout << "Display created" << endl;

	//al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
	
	this->timer = al_create_timer(1.0 / TICKS_PER_SECOND);
	this->event_queue = al_create_event_queue();

	#ifdef ALLEGRO_ANDROID
	al_register_event_source(event_queue, al_get_touch_input_event_source());
	#endif
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	al_register_event_source(event_queue, al_get_timer_event_source(timer));

	al_start_timer(timer);
	
}


int AllegroHandler::get_window_width() {
	
	return al_get_display_width(display);

}

int AllegroHandler::get_window_height() {
	
	return al_get_display_height(display);
	
}



void AllegroHandler::fit_display() {

	this->window_width = al_get_display_width(display);
	this->window_height = al_get_display_height(display);

	// calculate scaling factor
	float sx = float(window_width) / screen_width;
	float sy = float(window_height) / screen_height;

	this->scaled = std::min(sx, sy);

	#ifdef __ANDROID__
	if (screen_height * scaled > 0.85 * this->window_height) {
		this->scaled *= 0.85;
	}
	#endif

	// calculate how much the buffer should be scaled
	this->scale_w = screen_width * scaled;
	this->scale_h = screen_height * scaled;
	this->scale_x = (window_width - scale_w) / 2;
	this->scale_y = (window_height - scale_h) / 2;

	if (this->sec_buffer) {
		al_destroy_bitmap(this->sec_buffer);
	}

	this->sec_buffer = al_create_bitmap(window_width / this->scaled, window_height / this->scaled);
	

}


Point AllegroHandler::get_mapped_coordinates(int real_x, int real_y) {

	return {
		(int)((real_x - scale_x) / scaled),
		(int)((real_y - scale_y) / scaled)	
	};

}

void AllegroHandler::start_drawing() {

	al_set_target_backbuffer(display);
	al_clear_to_color(al_map_rgb(0, 0, 0));

}

void AllegroHandler::prepare_main_surface() {

	al_set_target_bitmap(buffer);
	al_clear_to_color(al_map_rgba(0, 0, 0, 0));
}

void AllegroHandler::draw_main_surface() {

	al_set_target_backbuffer(display);
	al_draw_scaled_bitmap(buffer, 0, 0, screen_width, screen_height, scale_x, scale_y, scale_w, scale_h, 0);

}

void AllegroHandler::prepare_sec_surface() {

	al_set_target_bitmap(sec_buffer);
	al_clear_to_color(al_map_rgba(0, 0, 0, 0));
}

void AllegroHandler::draw_sec_surface() {

	al_set_target_backbuffer(display);
	al_draw_scaled_bitmap(sec_buffer, 0, 0, window_width / this->scaled, window_height / this->scaled, 0, 0, this->window_width, this->window_height, 0);

}

void AllegroHandler::finish_drawing() {

	al_wait_for_vsync();
	al_flip_display();

}

float AllegroHandler::get_scaled() {

	return this->scaled;

}

void AllegroHandler::cleanup() {

	al_uninstall_audio();
	al_destroy_display(this->display);
	al_destroy_timer(this->timer);
	al_destroy_event_queue(this->event_queue);

}


//-----------------------------------------------------------------------------
//-------------------------------- [Stage] ------------------------------------

Stage::Stage(HGameEngine* _engine) {

	this->engine = _engine;

}

void Stage::on_event(ALLEGRO_EVENT event) {

}

void Stage::draw() {

}

void Stage::on_tick() {

}

void Stage::on_enter_stage() {

}


//-----------------------------------------------------------------------------
//----------------------------- [HGameEngine] ---------------------------------

HGameEngine::HGameEngine(): touch_keys(this) {

	string custom_cfg_filepath = "cfg.json";
	string default_cfg_filepath = "resources/defaultCfg.json";
	
	if (file_exists(custom_cfg_filepath)) {

		boost::json::value cfg_v = boost::json::parse(file_get_contents(custom_cfg_filepath));

		if (cfg_v.is_object()) {

			this->cfg = cfg_v.get_object();

		}

	} else if (file_exists(default_cfg_filepath)) {

		boost::json::value cfg_v = boost::json::parse(file_get_contents(default_cfg_filepath));

		if (cfg_v.is_object()) {

			this->cfg = cfg_v.get_object();

		}

	} else {

		this->cfg["windowed"] = false;
		this->cfg["defaultServer"] = "copinstar.com";
		this->cfg["defaultPort"] = 51009;

	}

	cout << "CFG: " << this->cfg << endl;

	this->allegro_hnd = new AllegroHandler(this); 
	
	cout << "Initializing resources..." << endl;
	this->allegro_hnd->initialize_resources();

	cout << "Creating components..." << endl;
	this->allegro_hnd->create_components();

	for (unsigned int i = 0; i < sizeof(keys); i++) {
		keys[i] = false;
	}

	font = al_load_ttf_font(FONT_DIR, scale * 10, ALLEGRO_TTF_MONOCHROME);

	stages[MENU] = new MainMenuStage(this);
	stages[GAME] = new GameStage(this);
	stages[OVER] = new GameOverStage(this);
	stages[CONN] = new ConnStage(this);
	stages[LOBBY] = new LobbyStage(this);

}


void HGameEngine::run() {

	cout << "Main loop starts" << endl;

	ALLEGRO_EVENT event;

	bool drawing_halted = false;
	
	do {

		al_wait_for_event(this->allegro_hnd->event_queue, &event);
		if (
			event.type == ALLEGRO_EVENT_KEY_CHAR || 
			event.type == ALLEGRO_EVENT_KEY_DOWN ||
			event.type == ALLEGRO_EVENT_KEY_UP ||
			event.type == ALLEGRO_EVENT_TOUCH_BEGIN ||
			event.type == ALLEGRO_EVENT_TOUCH_END ||
			event.type == ALLEGRO_EVENT_TOUCH_MOVE
		) {
			
			this->touch_keys.redefine_touch_event(event);

			if (event.keyboard.keycode == ALLEGRO_KEY_BACK) {
				//Bind back button to Escape key
				event.keyboard.keycode = ALLEGRO_KEY_ESCAPE;
			}
			this->on_event(event);
		}

		else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			break;
		}

		else if (event.type == ALLEGRO_EVENT_DISPLAY_HALT_DRAWING) {
			drawing_halted = true;
			al_acknowledge_drawing_halt(event.display.source);
		}

		else if (event.type == ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING) {
			drawing_halted = false;
			al_acknowledge_drawing_resume(event.display.source);
		}

		else if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
			al_acknowledge_resize(event.display.source);
			this->allegro_hnd->fit_display();
			this->touch_keys.re_arrange();
		}

		else if (event.type == ALLEGRO_EVENT_TIMER) {

			this->run_tick();

			if (!drawing_halted && al_is_event_queue_empty(this->allegro_hnd->event_queue)) {

				this->draw();

			}

		}

	} while (!this->finish);

	cout << "Cleaning up..." << endl;
	
	this->allegro_hnd->cleanup();

	cout << "Execution finished correctly." << endl;

}

void HGameEngine::calcFPS() {

	double game_time = al_get_time();

	if (game_time - old_time >= 1.0) {

		fps = ((float)frames_done) / (game_time - old_time);
		frames_done = 0;
		old_time = game_time;

	}

	frames_done++;

}

void HGameEngine::set_stage(unsigned int stage_id) {
	
	this->active_stage_id = stage_id;
	this->must_run_on_enter_stage = true;

	std::cout << " > set_stage: " << this->active_stage_id << std::endl;

}

void HGameEngine::run_tick() {

	//std::cout << "run_tick... ";

	auto active_stage = (Stage*) this->stages[this->active_stage_id];

	if (this->must_run_on_enter_stage) {
		active_stage->on_enter_stage();
		this->must_run_on_enter_stage = false;
	}

	active_stage->on_tick();

	//std::cout << "done!" << std::endl;

}

void HGameEngine::draw() {

	this->allegro_hnd->start_drawing();
	//std::cout << "drawing... ";
	this->allegro_hnd->prepare_main_surface();

	this->calcFPS();

	auto active_stage = (Stage*) this->stages[this->active_stage_id];

	active_stage->draw();

	//al_draw_text(font, al_map_rgb(255, 255, 0), 5, 20, ALLEGRO_ALIGN_LEFT, this->debug_txt.c_str());

	this->allegro_hnd->draw_main_surface();
	
	this->allegro_hnd->prepare_sec_surface();
	
	#ifdef ALLEGRO_ANDROID
	this->touch_keys.draw();
	#endif
	
	this->allegro_hnd->draw_sec_surface();

	this->allegro_hnd->finish_drawing();

	//std::cout << "done!" << std::endl;

}

void HGameEngine::on_event(ALLEGRO_EVENT event) {

	auto active_stage = (Stage*) this->stages[this->active_stage_id];

	if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
		
		keys[event.keyboard.keycode] = true;

	}

	else if (event.type == ALLEGRO_EVENT_KEY_UP) {
		
		keys[event.keyboard.keycode] = false;

	}

	active_stage->on_event(event);

}