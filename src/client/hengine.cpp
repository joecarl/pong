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

#include <iostream>

using namespace std;

//-----------------------------------------------------------------------------
//---------------------------- [AllegroHandler] -------------------------------

AllegroHandler::AllegroHandler(HGameEngine *gameEngine){
	
	this->engine = gameEngine;

}

void AllegroHandler::initializeResources(){

	al_init_image_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_primitives_addon();
	
	if(!al_init()) {
		throw std::runtime_error("failed to initialize allegro!");
	}
	if (!al_install_audio()) {
		throw std::runtime_error("could not init sound!");
	}
	if (!al_reserve_samples(1)) {
		throw std::runtime_error("could not reserve samples!"); 
	}
	if (!al_install_keyboard()) {
		throw std::runtime_error("could not init keyboard!");
	}

	cout << "Allegro initialized" << endl;

}

void AllegroHandler::createComponents(){

	bool windowed = this->engine->cfg["windowed"].as_bool();
	
	if(!windowed){
		al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
	}

	al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_REQUIRE);

	//ALLEGRO_DISPLAY_MODE disp_data;
		
	//al_get_display_mode(al_get_num_display_modes() - 1, &disp_data);

	this->screenWidth = this->engine->scale * this->engine->resX;
	this->screenHeight = this->engine->scale * this->engine->resY;

	display = al_create_display(640, 400);//disp_data.width, disp_data.height);
	if(!display){
		throw std::runtime_error("failed to create display!");
	}
	
	int windowWidth = al_get_display_width(display);
	int windowHeight = al_get_display_height(display);
	
	this->buffer = al_create_bitmap(screenWidth, screenHeight);

	// calculate scaling factor
	float sx = float(windowWidth) / screenWidth;
	float sy = float(windowHeight) / screenHeight;

	float scaled = std::min(sx, sy);

	// calculate how much the buffer should be scaled
	this->scaleW = screenWidth * scaled;
	this->scaleH = screenHeight * scaled;
	this->scaleX = (windowWidth - scaleW) / 2;
	this->scaleY = (windowHeight - scaleH) / 2;

	if(!windowed){
		al_hide_mouse_cursor(display);
	}
	
	cout << "Display created" << endl;

	/*
	for(int i = 0; i < al_get_num_display_modes(); i++){
		al_get_display_mode(i, &disp_data);
		cout << "Resolution: " << disp_data.width << "x" << disp_data.height << endl;
	}
	*/
	//al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
	
	this->timer = al_create_timer(1.0 / TICKS_PER_SECOND);
	this->event_queue = al_create_event_queue();

	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	al_register_event_source(event_queue, al_get_timer_event_source(timer));

	al_start_timer(timer);
	
}

void AllegroHandler::startDrawing(){

	al_set_target_bitmap(buffer);
	al_clear_to_color(al_map_rgb(0, 0, 0));
}

void AllegroHandler::finishDrawing(){
	
	al_set_target_backbuffer(display);
	al_clear_to_color(al_map_rgb(0, 0, 0));
	al_draw_scaled_bitmap(buffer, 0, 0, screenWidth, screenHeight, scaleX, scaleY, scaleW, scaleH, 0);
	
	al_wait_for_vsync();
	al_flip_display();

}

void AllegroHandler::cleanup(){

	al_uninstall_audio();
	al_destroy_display(this->display);
	al_destroy_timer(this->timer);
	al_destroy_event_queue(this->event_queue);

}


//-----------------------------------------------------------------------------
//-------------------------------- [Stage] ------------------------------------

Stage::Stage(HGameEngine* _engine){

	this->engine = _engine;

}

void Stage::onEvent(ALLEGRO_EVENT event){

}

void Stage::draw(){

}

void Stage::onTick(){

}

void Stage::onEnterStage(){

}


//-----------------------------------------------------------------------------
//----------------------------- [HGameEngine] ---------------------------------

HGameEngine::HGameEngine(){

	string customCfgFilePath = "cfg.json";
	string defaultCfgFilePath = "resources/defaultCfg.json";
	
	if(file_exists(customCfgFilePath)){

		boost::json::value cfgV = boost::json::parse(file_get_contents(customCfgFilePath));

		if(cfgV.is_object()){

			this->cfg = cfgV.get_object();

		}

	} else if(file_exists(defaultCfgFilePath)){

		boost::json::value cfgV = boost::json::parse(file_get_contents(defaultCfgFilePath));

		if(cfgV.is_object()){

			this->cfg = cfgV.get_object();

		}

	} else {

		this->cfg["windowed"] = false;
		this->cfg["defaultServer"] = "copinstar.com";
		this->cfg["defaultPort"] = 28090;

	}

	//cout << this->cfg << endl;

	this->allegroHnd = new AllegroHandler(this); 
	
	this->allegroHnd->initializeResources();

	this->allegroHnd->createComponents();

	for (unsigned int i = 0; i < sizeof(keys); i++){
		keys[i] = 0;
	}

	font = al_load_ttf_font(FONT_DIR, scale * 10, ALLEGRO_TTF_MONOCHROME);

	stages[MENU] = new MainMenuStage(this);
	stages[GAME] = new GameStage(this);
	stages[OVER] = new GameOverStage(this);
	stages[CONN] = new ConnStage(this);
	stages[LOBBY] = new LobbyStage(this);

}


void HGameEngine::run(){

	cout << "Main loop starts" << endl;

	ALLEGRO_EVENT event;
	
	do{

		al_wait_for_event(this->allegroHnd->event_queue, &event);
		if(
			event.type == ALLEGRO_EVENT_KEY_CHAR || 
			event.type == ALLEGRO_EVENT_KEY_DOWN ||
			event.type == ALLEGRO_EVENT_KEY_UP
		){
			this->onEvent(event);
		}

		else if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE){
			break;
		}

		else if(event.type == ALLEGRO_EVENT_TIMER){

			this->runTick();

			if(al_event_queue_is_empty(this->allegroHnd->event_queue)){
				
				this->allegroHnd->startDrawing();
				
				this->draw();

				this->allegroHnd->finishDrawing();

			}

		}

	} while(!this->finish);

	cout << "Execution finished by user" << endl;
	al_rest(0.5);

	this->allegroHnd->cleanup();

}

void HGameEngine::calcFPS(){

	double game_time = al_get_time();

	if(game_time - old_time >= 1.0){

		fps = ((float)frames_done) / (game_time - old_time);
		frames_done = 0;
		old_time = game_time;

	}

	frames_done++;

}

void HGameEngine::setStage(unsigned int stageID){
	
	this->activeStageID = stageID;
	this->mustRunOnEnterStage = true;

	std::cout << " > setStage: " << this->activeStageID << std::endl;

}

void HGameEngine::runTick(){

	//std::cout << "runTick... ";

	Stage* activeStage = (Stage*) this->stages[this->activeStageID];

	if(this->mustRunOnEnterStage){
		activeStage->onEnterStage();
		this->mustRunOnEnterStage = false;
	}

	activeStage->onTick();

	//std::cout << "done!" << std::endl;

}

void HGameEngine::draw(){
	
	//std::cout << "drawing... ";

	this->calcFPS();
	
	Stage* activeStage = (Stage*) this->stages[this->activeStageID];

	activeStage->draw();

	//std::cout << "done!" << std::endl;

}

void HGameEngine::onEvent(ALLEGRO_EVENT event){

	Stage* activeStage = (Stage*) this->stages[this->activeStageID];

	if(event.type == ALLEGRO_EVENT_KEY_DOWN){
		
		keys[event.keyboard.keycode] = true;

	}

	else if(event.type == ALLEGRO_EVENT_KEY_UP){
		
		keys[event.keyboard.keycode] = false;

	}

	activeStage->onEvent(event);

}