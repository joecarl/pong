
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <boost/json.hpp>

#include "hengine.h"
#include "classes.h"
#include "stages.h"

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


HGameEngine::HGameEngine(){

	for (unsigned int i = 0; i < sizeof(keys); i++){
		keys[i] = 0;
	}

	font = al_load_ttf_font(FONT_DIR, scale*10, ALLEGRO_TTF_MONOCHROME);

	pongGame = new PongGame(scale, font);

	stages[MENU] = new MainMenuStage(this);
	stages[GAME] = new GameStage(this);
	stages[OVER] = new GameOverStage(this);
	stages[CONN] = new ConnStage(this);

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

	boost::json::value inputEvt;
	inputEvt.emplace_null();

	if(event.type == ALLEGRO_EVENT_KEY_DOWN){
		inputEvt = {
			{"type", event.type},
			{"keycode", event.keyboard.keycode}
		};
		//cout << inputEvt << endl;
		keys[event.keyboard.keycode] = true;
	}

	else if(event.type == ALLEGRO_EVENT_KEY_UP){
		inputEvt = {
			{"type", event.type},
			{"keycode", event.keyboard.keycode}
		};
		//cout << inputEvt << endl;
		keys[event.keyboard.keycode] = false;
	}

	activeStage->onEvent(event);

}