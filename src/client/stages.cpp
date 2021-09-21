#include "../ponggame.h"
#include "../utils.h"
#include "mediatools.h"
#include "hengine.h"
#include "stages.h"
#include "onlinestages.h"

#include <iostream>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <boost/json.hpp>
#include <string>
#include <math.h>

using namespace std;


void GameHandler::setup(int _playMode, int _controlMode){

	this->playMode = _playMode;
	this->controlMode = _controlMode;
	
}

void GameHandler::makeNewPongGame(int_fast32_t seed){

	this->cleanup();

	pongGame = new PongGame(seed);

	pongGame->controlMode = this->controlMode;

}

void GameHandler::cleanup(){

	if(pongGame != nullptr){
		delete pongGame;
	}

}

GameHandler::~GameHandler(){

	this->cleanup();

}

/**
 * Retrieves the game CONTROL_* based on controlMode, keycode and playerID
 */
int GameHandler::getControl(int kCode, int playerID){

	if(this->pongGame->controlMode == CONTROLMODE_TWO_PLAYERS && this->playMode == PLAYMODE_LOCAL) {

		if(playerID == 1){

			if (kCode == ALLEGRO_KEY_UP || kCode == ALLEGRO_KEY_I) return CONTROL_MOVE_UP;
			else if (kCode == ALLEGRO_KEY_DOWN || kCode == ALLEGRO_KEY_K) return CONTROL_MOVE_DOWN;

		} else if(playerID == 0){

			if (kCode == ALLEGRO_KEY_W) return CONTROL_MOVE_UP;
			else if (kCode == ALLEGRO_KEY_S) return CONTROL_MOVE_DOWN;

		}

	} else {

		if (kCode == ALLEGRO_KEY_UP) return CONTROL_MOVE_UP;
		else if (kCode == ALLEGRO_KEY_DOWN) return CONTROL_MOVE_DOWN;

	}

	return CONTROL_NONE;

}



GameHandler gameHandler;



//-----------------------------------------------------------------------------
//------------------------------ MainMenuStage --------------------------------

MainMenuStage::MainMenuStage(HGameEngine* _engine):Stage(_engine){
	
	this->logo = load_bitmap(LOGO_DIR);

	//std::cout << "MainMenuStage init" << std::endl;

}


void MainMenuStage::onEnterStage(){

	this->engine->touchKeys.clearButtons();

	this->engine->touchKeys.addButton(ALLEGRO_KEY_1, "1");
	this->engine->touchKeys.addButton(ALLEGRO_KEY_2, "2");
	this->engine->touchKeys.addButton(ALLEGRO_KEY_3, "3");
	this->engine->touchKeys.addButton(ALLEGRO_KEY_4, "4");
	this->engine->touchKeys.addButton(ALLEGRO_KEY_ESCAPE, "ESC");

	this->engine->touchKeys.fitButtons(FIT_BOTTOM, 10);
}


void MainMenuStage::onTick(){

	if(easteregg++ == 5000) PlayExorcista();

}
	

void MainMenuStage::onEvent(ALLEGRO_EVENT event){

	int keycode = event.keyboard.keycode;

	if(event.type == ALLEGRO_EVENT_KEY_DOWN){
	
		if(keycode == ALLEGRO_KEY_1){
			gameHandler.setup(PLAYMODE_LOCAL, CONTROLMODE_SINGLE_PLAYER);
		} 
		else if(keycode == ALLEGRO_KEY_2) {
			gameHandler.setup(PLAYMODE_LOCAL, CONTROLMODE_TWO_PLAYERS);
		}
		else if(keycode == ALLEGRO_KEY_3){
			gameHandler.setup(PLAYMODE_LOCAL, CONTROLMODE_TRAINING);
		}
		else if(keycode == ALLEGRO_KEY_4){
			this->engine->setStage(CONN); 
			gameHandler.setup(PLAYMODE_ONLINE, CONTROLMODE_TWO_PLAYERS);
		}
		else if(keycode == ALLEGRO_KEY_5){
			gameHandler.setup(PLAYMODE_LOCAL, CONTROLMODE_DEBUG);
		}
		
		if(keycode == ALLEGRO_KEY_C){//CAMBIAR RESOLUCION, bad performance
			/*
			scale = scale == 1 ? 2 : 1;
			al_destroy_font(font);
			al_destroy_bitmap(buffer);
			font = al_load_ttf_font("resources/font.ttf", scale * 9, 0) ;
			buffer = al_create_bitmap(scale * resX, scale * resY);
			*/
		}

		if(keycode == ALLEGRO_KEY_ESCAPE){
			this->engine->finish = true;
		}

		else if(keycode == ALLEGRO_KEY_1 || keycode == ALLEGRO_KEY_2 || keycode == ALLEGRO_KEY_3 || keycode == ALLEGRO_KEY_5) {

			gameHandler.makeNewPongGame(time(nullptr)); 
			this->engine->setStage(GAME);

		}
	}

}


void MainMenuStage::draw(){

	float sc = this->engine->scale;
	ALLEGRO_FONT* font = this->engine->font;

	al_draw_bitmap(this->logo, (sc - 1) * DEF_W / 2, (sc - 1) * 50, 0);
	al_draw_text(font, al_map_rgb( 255, 255, 255), sc * DEF_W / 2, sc * 105, ALLEGRO_ALIGN_CENTER, "Recreated by: Jose Carlos HR");
	al_draw_text(font, al_map_rgb( 255, 255, 255), sc * DEF_W / 2, sc * 130, ALLEGRO_ALIGN_CENTER, "1:One Player  2:Two Players");
	al_draw_text(font, al_map_rgb( 255, 255, 255), sc * DEF_W / 2, sc * 140, ALLEGRO_ALIGN_CENTER, "3:Training    4:Play online");
	al_draw_text(font, al_map_rgb( 255, 255, 255), sc * DEF_W / 2, sc * 155, ALLEGRO_ALIGN_CENTER, "ESC: Quit");

	////al_draw_textf(font,WHITE,20, 75, ALLEGRO_ALIGN_LEFT,"%f", sin(x)*sin(x));
	
}


//-----------------------------------------------------------------------------
//------------------------------- [ GameStage ] -------------------------------


GameStage::GameStage(HGameEngine* _engine):Stage(_engine){
   
	this->tracer = new Tracer(_engine);

}

void GameStage::drawCourt(){

	float scale = this->engine->scale;

	float minCourtY = scale * (LIMIT);
	float maxCourtY = scale * (MAX_Y - LIMIT);

	al_draw_line(0, minCourtY, scale*DEF_W, minCourtY, al_map_rgb( 255, 255, 255),2);
	al_draw_line(0, maxCourtY,scale*DEF_W, maxCourtY, al_map_rgb( 255, 255, 255),2);
	al_draw_line(scale*(320/2-1), minCourtY, scale*(320/2-1), maxCourtY, al_map_rgb( 255, 255, 255),2);
	al_draw_line(scale*(320/2+1), minCourtY, scale*(320/2+1), maxCourtY, al_map_rgb( 255, 255, 255),2);

	if(gameHandler.pongGame->controlMode == CONTROLMODE_TRAINING){
		
		al_draw_line(scale*DEF_W, minCourtY, scale*DEF_W, maxCourtY, al_map_rgb( 255, 255, 255), 2);

	}

}

void GameStage::drawScores(){
	
	ALLEGRO_FONT* font = this->engine->font;
	float scale = this->engine->scale;

	if(gameHandler.pongGame->controlMode != CONTROLMODE_TRAINING){
		al_draw_textf( font, al_map_rgb( 255, 0, 0), scale*25, scale*186, ALLEGRO_ALIGN_LEFT, "SCORE:%d", gameHandler.pongGame->players[0]->score );
		al_draw_textf( font, al_map_rgb( 255, 0, 0), scale*240, scale*186, ALLEGRO_ALIGN_LEFT, "SCORE:%d", gameHandler.pongGame->players[1]->score );
	} else {
		al_draw_textf( font, al_map_rgb( 255, 0, 0), scale*25, scale*186, ALLEGRO_ALIGN_LEFT, "FAILS:%d", gameHandler.pongGame->players[1]->score );
	}

}

void GameStage::onEnterStage(){
	
	this->engine->touchKeys.clearButtons();

	this->engine->touchKeys.addButton(ALLEGRO_KEY_DOWN, "");
	this->engine->touchKeys.addButton(ALLEGRO_KEY_UP, "");

	this->engine->touchKeys.fitButtons(FIT_HORIZONTAL);


	if(gameHandler.playMode == PLAYMODE_ONLINE){

		this->engine->connection.process_actions_fn = [&](boost::json::object& evt){

			cout << "QUEUED: " << evt << endl;
			controller.evt_queue.push(evt);
			
		};

	}

	PlaySound(Do, 400);
	PlaySound(Re, 200);
	PlaySound(La, 100);
	PlaySound(Si, 100);
	PlayAudio();

	gameHandler.pongGame->restart();
	gameHandler.pongGame->iniciarPunto(1);

	delayer = 75;

}


void GameStage::onEvent(ALLEGRO_EVENT evt){

	int kCode = evt.keyboard.keycode;
	
	if(evt.type == ALLEGRO_EVENT_KEY_DOWN){

		if(kCode == ALLEGRO_KEY_ESCAPE){

			this->engine->setStage(MENU);

		}

		else if(kCode == ALLEGRO_KEY_P){//P (PAUSA)
			
			gameHandler.pongGame->togglePause();

		}

	}

	if(evt.type == ALLEGRO_EVENT_KEY_DOWN || evt.type == ALLEGRO_EVENT_KEY_UP){

		bool newSt = evt.type == ALLEGRO_EVENT_KEY_DOWN;

		int controlP1 = gameHandler.getControl(kCode, 0);
		int controlP2 = gameHandler.getControl(kCode, 1);

		if(gameHandler.playMode == PLAYMODE_ONLINE){
				
			if(controlP1 != CONTROL_NONE){

				boost::json::value inputEvt;
				//inputEvt.emplace_null();
			
				inputEvt = {
					{"type", "set_control_state"},
					{"state", newSt},
					{"control", controlP1},
					{"tick", gameHandler.pongGame->tick}
				};

				cout << "Sending: " << inputEvt << endl;
				this->engine->connection.qsend(boost::json::serialize(inputEvt));//send inputEvt

			}

		} else {
		
			gameHandler.pongGame->players[0]->controls[controlP1] = newSt;

			gameHandler.pongGame->players[1]->controls[controlP2] = newSt;
			
		} 

	}
	
}


void GameStage::processMessage(string &msg){

	if(msg == "scored"){

		PlaySound(Re, 130);
		PlaySound(Do, 250);
		PlayAudio();

	}
	else if(msg == "hit"){

		PlaySound(Mi, 40, 4);
		PlayAudio();

	}

}


void GameStage::onTick(){

	if (delayer > 0){
		delayer--;
		return;
	} 

	if(gameHandler.pongGame->paused) {
		return;
	}

	if(gameHandler.playMode == PLAYMODE_ONLINE){

		controller.onTick();
	
	} else {

		if(this->engine->keys[ALLEGRO_KEY_G]) {
			//gameHandler.pongGame->players[0]->medlen += 1;//DEBUG
		}

	}

	gameHandler.pongGame->processTick();

	while(!gameHandler.pongGame->messages.empty()){
		string msg = gameHandler.pongGame->messages.front();
		gameHandler.pongGame->messages.pop();
		this->processMessage(msg);
	}

	if(gameHandler.pongGame->finished){
		
		this->engine->setStage(OVER);
		PlaySound(Re, 150, 3);
		PlaySound(Re, 150, 3);
		PlaySound(Re, 200, 3);
		PlaySound(LaSos, 500, 2);
		PlayAudio();

	}
	
}


void GameStage::draw(){

	ALLEGRO_FONT* font = this->engine->font;

	float scale = this->engine->scale;
	
	if (delayer > 0) {

		al_draw_textf(
			font, 
			al_map_rgb(255, 0, 0), 
			scale * this->engine->resX / 2, 
			scale * this->engine->resY / 2, 
			ALLEGRO_ALIGN_CENTER, 
			"%d", 
			1 + delayer / 25
		);
		
	} else {

		if (gameHandler.pongGame->paused) {

			al_draw_text(
				font, 
				al_map_rgb(0, 200, 100), 
				this->engine->resX * scale / 2, 
				this->engine->resY * scale / 2 - 5, 
				ALLEGRO_ALIGN_CENTER, 
				"PAUSA"
			);
			
		} else {

			//al_draw_text(font, al_map_rgb(255, 0, 0), scale * 320 / 2, scale * 2, ALLEGRO_ALIGN_CENTER, "Press ESC to Main Menu");
			
			int secs = gameHandler.pongGame->tick / 60;
			int secsD = (secs % 60) / 10;
			int secsU = secs % 10;
			int min = secs / 60;
			al_draw_textf(font, al_map_rgb(255, 0, 0), scale * 320 / 2, scale * 1, ALLEGRO_ALIGN_CENTER, "%d:%d%d", min, secsD, secsU);
				
			this->drawCourt();

			Tracer *tr = this->tracer;

			tr->drawBall(gameHandler.pongGame->ball, scale);
			tr->drawBonus(gameHandler.pongGame->bonus[0], scale);
			tr->drawBonus(gameHandler.pongGame->bonus[1], scale);

			this->drawScores();

			al_draw_textf(font, al_map_rgb(255, 0, 0), scale * 160, scale * 186, ALLEGRO_ALIGN_CENTER, "FPS: %d", (int)(this->engine->fps));

			if(gameHandler.playMode == PLAYMODE_ONLINE){
				al_draw_textf(font, al_map_rgb(255, 0, 0), scale * (320 - 4), scale * 1, ALLEGRO_ALIGN_RIGHT, "PING: %d", (int)(this->engine->connection.ping_ms));
			}

			tr->drawPlayer(gameHandler.pongGame->players[0], scale);
			if(gameHandler.pongGame->controlMode != CONTROLMODE_TRAINING){
				tr->drawPlayer(gameHandler.pongGame->players[1], scale);
			}

		}

	}

}



//-----------------------------------------------------------------------------
//---------------------------------[ TRACER ]----------------------------------


Tracer::Tracer(HGameEngine* _engine){

	this->engine = _engine;

	this->bonus_ball_spr = load_bitmap(BALL_DIR);

	this->bonus_long_spr = load_bitmap(LONG_DIR);

}

ALLEGRO_BITMAP* Tracer::getSpriteForBonusType(int bonus_type){

	if(bonus_type == BONUS_BALL) {

		return this->bonus_ball_spr;

	} else if(bonus_type == BONUS_LONG) {

		return this->bonus_long_spr;

	} else {

		cerr << "Unknown bonus type: " << bonus_type << endl;

		return nullptr;

	}

}

void Tracer::drawBall(Ball *b, float scale){
		
	if(!b->stat){
		return;
	}

	al_draw_filled_circle(scale * b->x, scale * b->y, scale * b->radius, al_map_rgb( 255,255, 255));

}

void Tracer::drawBonus(Bonus * b, float scale){
	
	if(!b->stat){
		return;
	}

	ALLEGRO_BITMAP *spr = this->getSpriteForBonusType(b->bonus_type);
	if(spr != nullptr){
		al_draw_bitmap(spr, scale * (b->x - 10), scale * (b->y - 10), 0);
		// x-sprite->w/2, y-sprite->h/2);
	}

}


void Tracer::drawPlayer(PlayerP *pl, int scale){

	int medln = pl->medlen;
	
	al_draw_filled_rectangle(
		scale * (pl->x - 2), 
		scale * (pl->y - medln), 
		scale * (pl->x + 2), 
		scale * (pl->y + medln), 
		WHITE
	);

	al_draw_filled_rectangle(
		scale * (pl->x - 1),
		scale * (pl->y - medln - 1), 
		scale * (pl->x + 1), 
		scale * (pl->y + medln + 1), 
		WHITE
	);
	
	if(pl->bonus_timers[BONUS_BALL]){

		if(pl->x < 100){

			al_draw_filled_rectangle(
				scale * 0, 
				scale * 16, 
				scale * (158.0 * pl->bonus_timers[BONUS_BALL] / 800.0), 
				scale * 20, 
				al_map_rgb(0, 200, 50)
			);

		} else {

			al_draw_filled_rectangle(
				scale * (DEF_W), 
				scale * 16, 
				scale * (DEF_W - 158.0 * pl->bonus_timers[BONUS_BALL] / 800.0),
				scale * 20, 
				al_map_rgb(0, 200, 50)
			);

		}
	
	}

	if(pl->comTxtY >- 40){

		int txtX;

		if(pl->x < 100) txtX = 60;
		else txtX = 220;

		al_draw_text(
			this->engine->font, 
			al_map_rgb(  150 - pl->comTxtY, 150 - pl->comTxtY * 4, 255), 
			scale * txtX, 
			scale * pl->comTxtY, 
			ALLEGRO_ALIGN_CENTER, 
			pl->comTxt.c_str()
		);

		pl->comTxtY -= 2;
	}

}



//-----------------------------------------------------------------------------
//------------------------------- GameOverStage -------------------------------


void GameOverStage::onEnterStage(){

	this->engine->touchKeys.clearButtons();

	this->engine->touchKeys.addButton(ALLEGRO_KEY_Y, "Y");
	this->engine->touchKeys.addButton(ALLEGRO_KEY_N, "N");

	this->engine->touchKeys.fitButtons(FIT_BOTTOM, 10);
	
}

void GameOverStage::onEvent(ALLEGRO_EVENT event){

	if(event.type == ALLEGRO_EVENT_KEY_DOWN){

		int keycode = event.keyboard.keycode;

		if(keycode == ALLEGRO_KEY_Y){
			if(gameHandler.playMode == PLAYMODE_ONLINE){

				this->engine->setStage(LOBBY);

			} else {
				
				this->engine->setStage(GAME);

			}
		} 
		else if(keycode == ALLEGRO_KEY_N){
			this->engine->setStage(MENU);
		}

	}

}

void GameOverStage::draw(){

	int winner;
	if(gameHandler.pongGame->players[0]->score > gameHandler.pongGame->players[1]->score) winner = 1;
	else winner = 2;

	float scale = this->engine->scale;
	ALLEGRO_FONT *font = this->engine->font;

	al_draw_textf(font, al_map_rgb( 255, 0, 0), scale*320/2, scale*54, ALLEGRO_ALIGN_CENTER, "WINNER: PLAYER %d", winner);
	al_draw_text (font, al_map_rgb( 255, 0, 0), scale*320/2, scale*70, ALLEGRO_ALIGN_CENTER, "REPLAY? (Y/N)" );

}
