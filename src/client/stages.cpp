#include "../classes.h"
#include "../utils.h"
#include "mediatools.h"
#include "hengine.h"
#include "stages.h"

#include <iostream>
#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <boost/json.hpp>
#include <string>
#include <math.h>

using namespace std;

enum{
	PLAYMODE_NONE = 0,
	PLAYMODE_SINGLE_PLAYER,
	PLAYMODE_TWO_PLAYERS,
	PLAYMODE_TRAINING,
	PLAYMODE_ONLINE
};

int playMode = PLAYMODE_NONE;

PongGame* pongGame = nullptr;

void makeNewPongGame(int_fast32_t seed){

	if(pongGame != nullptr){
		delete pongGame;
	}

	pongGame = new PongGame(seed);

	if(playMode == PLAYMODE_SINGLE_PLAYER){
		pongGame->numPlayers = 1;
	} 
	else if(playMode == PLAYMODE_TRAINING){
		pongGame->numPlayers = 0;
	}
	else if(playMode == PLAYMODE_ONLINE || playMode == PLAYMODE_TWO_PLAYERS){
		pongGame->numPlayers = 2;
	}
	
}


class Controller{

	PongGame *game;

	void process_event(boost::json::object &evt){

		cout << "Processing evt" << evt << endl;

		auto evtType = evt["type"].as_string();
			
		if(evtType == "set_control_state"){
			
			int control = evt["control"].as_int64();
			bool newState = evt["state"].as_bool();
			int playerID = evt["playerKey"].as_int64();
			
			this->game->players[playerID]->controls[control] = newState;

		} else {
			cerr << "Unknown event type: " << evtType << endl;
		}

	}

public:

	std::queue<boost::json::object> evt_queue;

	void setup(PongGame *game){
		
		this->game = game;
		
		//vaciamos la cola de eventos
		std::queue<boost::json::object> empty;
   		std::swap( this->evt_queue, empty );

	}

	void onTick(){

		while(this->evt_queue.size() > 0){

			auto evt = this->evt_queue.front();
			unsigned int evtTick = (unsigned int)evt["tick"].as_int64();

			if(evtTick == this->game->tick){

				this->process_event(evt);

				this->evt_queue.pop();

			} else if (evtTick < this->game->tick){

				cerr << "evtTick: " << evtTick << " | gameTick: " << this->game->tick << endl;

				throw std::runtime_error("Evento perdido");

			} else {
				
				break;

			}

		}
	}
};

Controller controller;


//-----------------------------------------------------------------------------
//------------------------------ MainMenuStage --------------------------------

MainMenuStage::MainMenuStage(HGameEngine* _engine):Stage(_engine){
	
	this->logo = al_load_bitmap(LOGO_DIR);

	//std::cout << "MainMenuStage init" << std::endl;

}

void MainMenuStage::onTick(){

	if(easteregg++ == 1000) PlayExorcista();

}
	

void MainMenuStage::onEvent(ALLEGRO_EVENT event){

	int keycode = event.keyboard.keycode;

	if(event.type == ALLEGRO_EVENT_KEY_DOWN){
	
		if(keycode == ALLEGRO_KEY_1){
			playMode = PLAYMODE_SINGLE_PLAYER;
		} 
		else if(keycode == ALLEGRO_KEY_2) {
			playMode = PLAYMODE_TWO_PLAYERS;
		}
		else if(keycode == ALLEGRO_KEY_3){
			playMode = PLAYMODE_TRAINING;
		}
		else if(keycode == ALLEGRO_KEY_4){
			this->engine->setStage(CONN); 
			playMode = PLAYMODE_ONLINE;
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

		else if(keycode == ALLEGRO_KEY_1 || keycode == ALLEGRO_KEY_2 || keycode == ALLEGRO_KEY_3) {

			makeNewPongGame(time(nullptr)); 
			this->engine->setStage(GAME);

		}
	}

}

void MainMenuStage::draw(){

	float sc = this->engine->scale;
	ALLEGRO_FONT* font = this->engine->font;

	al_draw_bitmap(this->logo, (sc - 1) * DEF_W / 2, (sc - 1) * 50, 0);
	al_draw_text(font, al_map_rgb( 255, 255, 255), sc * DEF_W / 2, sc * 110, ALLEGRO_ALIGN_CENTER, "Recreated by: Jose Carlos HR");
	al_draw_text(font, al_map_rgb( 255, 255, 255), sc * DEF_W / 2, sc * 140, ALLEGRO_ALIGN_CENTER, "1:One Player  2:Two Players");
	al_draw_text(font, al_map_rgb( 255, 255, 255), sc * DEF_W / 2, sc * 150, ALLEGRO_ALIGN_CENTER, "3:Training    4:Play online");
	al_draw_text(font, al_map_rgb( 255, 255, 255), sc * DEF_W / 2, sc * 165, ALLEGRO_ALIGN_CENTER, "ESC: Quit");

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

	if(playMode == PLAYMODE_TRAINING){
		
		al_draw_line(scale*DEF_W, minCourtY, scale*DEF_W, maxCourtY, al_map_rgb( 255, 255, 255), 2);

	}

}

void GameStage::drawScores(){
	
	ALLEGRO_FONT* font = this->engine->font;
	float scale = this->engine->scale;

	if(playMode != PLAYMODE_TRAINING){
		al_draw_textf( font, al_map_rgb( 255, 0, 0), scale*25, scale*186, ALLEGRO_ALIGN_LEFT, "SCORE:%d", pongGame->players[0]->score );
		al_draw_textf( font, al_map_rgb( 255, 0, 0), scale*240, scale*186, ALLEGRO_ALIGN_LEFT, "SCORE:%d", pongGame->players[1]->score );
	} else {
		al_draw_textf( font, al_map_rgb( 255, 0, 0), scale*25, scale*186, ALLEGRO_ALIGN_LEFT, "FAILS:%d", pongGame->players[1]->score );
	}

}

void GameStage::onEnterStage(){

	if(playMode == PLAYMODE_ONLINE){

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

	pongGame->restart();
	pongGame->iniciarPunto(1);

	delayer = 75;

}

int getControl(int kCode, int playMode, int playerID){

	if(playMode == PLAYMODE_TWO_PLAYERS) {

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

void GameStage::onEvent(ALLEGRO_EVENT evt){

	int kCode = evt.keyboard.keycode;
	
	if(evt.type == ALLEGRO_EVENT_KEY_DOWN){

		if(kCode == ALLEGRO_KEY_ESCAPE){

			this->engine->setStage(MENU);

		}

		else if(kCode == ALLEGRO_KEY_P){//P (PAUSA)
			
			pongGame->togglePause();

		}

	}

	if(evt.type == ALLEGRO_EVENT_KEY_DOWN || evt.type == ALLEGRO_EVENT_KEY_UP){

		bool newSt = evt.type == ALLEGRO_EVENT_KEY_DOWN;

		int controlP1 = getControl(kCode, playMode, 0);
		int controlP2 = getControl(kCode, playMode, 1);

		if(playMode == PLAYMODE_ONLINE){
				
			boost::json::value inputEvt;
			inputEvt.emplace_null();
			

			if(controlP1 != CONTROL_NONE){

				inputEvt = {
					{"type", "set_control_state"},
					{"state", newSt},
					{"control", controlP1}
				};

				cout << "Sending: " << inputEvt << endl;
				this->engine->connection.qsend(boost::json::serialize(inputEvt));//send inputEvt

			}

		} else {
		
			pongGame->players[0]->controls[controlP1] = newSt;

			pongGame->players[1]->controls[controlP2] = newSt;
			
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

	if(pongGame->paused) {
		return;
	}

	//if(keys[ALLEGRO_KEY_G]) players[0]->medlen += 1;//DEBUG
	if(playMode == PLAYMODE_ONLINE){

		controller.onTick();
	
	}

	pongGame->processTick();

	while(!pongGame->messages.empty()){
		string msg = pongGame->messages.front();
		pongGame->messages.pop();
		this->processMessage(msg);
	}

	if(pongGame->finished){
		
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

		if (pongGame->paused) {

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
				
			this->drawCourt();

			Tracer *tr = this->tracer;

			tr->drawBall(pongGame->ball, scale);
			tr->drawBonus(pongGame->bonus[0], scale);
			tr->drawBonus(pongGame->bonus[1], scale);

			this->drawScores();

			al_draw_textf(font, al_map_rgb(255, 0, 0), scale * 160, scale * 186, ALLEGRO_ALIGN_CENTER, "FPS: %d", (int)(this->engine->fps));

			if(playMode == PLAYMODE_ONLINE){
				al_draw_textf(font, al_map_rgb(255, 0, 0), scale * 320 / 2, scale * 2, ALLEGRO_ALIGN_CENTER, "PING:%d", (int)(this->engine->connection.ping_ms));
			}

			tr->drawPlayer(pongGame->players[0], scale);
			if(playMode != PLAYMODE_TRAINING){
				tr->drawPlayer(pongGame->players[1], scale);
			}

		}

	}

}



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
	
	if(pl->bonus_ball){

		if(pl->x < 100){

			al_draw_filled_rectangle(
				scale * 0, 
				scale * 16, 
				scale * (158.0 * pl->bonus_ball / 80.0), 
				scale * 20, 
				al_map_rgb(0, 200, 50)
			);

		} else {

			al_draw_filled_rectangle(
				scale * (DEF_W), 
				scale * 16, 
				scale * (DEF_W - 158.0 * pl->bonus_ball / 80.0),
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

	if(playMode == PLAYMODE_ONLINE){ //una copia de connstage, ver como optimizar... quiza moviendo a un lobbystage

		this->engine->connection.process_actions_fn = [&](boost::json::object& evt){

			cout << "RECEIVED: " << evt << endl;
			if(evt["type"] == "game_start"){

				makeNewPongGame((int_fast32_t)evt["seed"].as_int64());

				controller.setup(pongGame);

				this->engine->setStage(GAME);
			
			}

		};
	}
}

void GameOverStage::onEvent(ALLEGRO_EVENT event){

	if(event.type == ALLEGRO_EVENT_KEY_DOWN){

		int keycode = event.keyboard.keycode;

		if(keycode == ALLEGRO_KEY_Y){
			if(playMode == PLAYMODE_ONLINE){

				boost::json::object pkg = {{"type", "play_again"}};
				
				this->engine->connection.qsend(boost::json::serialize(pkg));

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
	if(pongGame->players[0]->score > pongGame->players[1]->score) winner = 1;
	else winner = 2;

	float scale = this->engine->scale;
	ALLEGRO_FONT *font = this->engine->font;

	al_draw_textf(font, al_map_rgb( 255, 0, 0), scale*320/2, scale*54, ALLEGRO_ALIGN_CENTER, "WINNER: PLAYER %d", winner);
	al_draw_text (font, al_map_rgb( 255, 0, 0), scale*320/2, scale*70, ALLEGRO_ALIGN_CENTER, "REPLAY? (Y/N)" );

}


//-----------------------------------------------------------------------------
//------------------------------- [ ConnStage ] -------------------------------

ConnStage::ConnStage(HGameEngine* _engine):Stage(_engine){

	this->input = new JC_TEXTINPUT(this->engine->font);

}

void ConnStage::onEnterStage(){

	input->start();
	//al_flush_event_queue(event_queue);
	//connection->Reset();

	this->engine->connection.process_actions_fn = [&](boost::json::object& evt){

		cout << "RECEIVED: " << evt << endl;
		if(evt["type"] == "game_start"){

			makeNewPongGame((int_fast32_t)evt["seed"].as_int64());

			controller.setup(pongGame);

			this->engine->setStage(GAME);
		
		}

	};

	
}

void ConnStage::onEvent(ALLEGRO_EVENT event){

	if(event.type == ALLEGRO_EVENT_KEY_CHAR){

		if(input->active){
			
			if (event.keyboard.keycode != ALLEGRO_KEY_ENTER) {

				input->processKey(event.keyboard.unichar, event.keyboard.keycode);

			} else {

				input->finish();
				server = input->getValue();

				if (server == ""){

					server = this->engine->cfg["defaultServer"].as_string().c_str();

				}

				unsigned short port = (unsigned short) this->engine->cfg["defaultPort"].as_int64();
							
				this->engine->connection.connect(server, port);
				
			}

		}

	}
	else if(event.type == ALLEGRO_EVENT_KEY_DOWN){

		int keycode = event.keyboard.keycode;

		if(keycode == ALLEGRO_KEY_ESCAPE){//ESC (SALIR)
			al_rest(0.2);
			input->active = false;
			this->engine->setStage(MENU);
		}

	}

}

void ConnStage::onTick(){


}

void ConnStage::draw(){

	ALLEGRO_FONT* font = this->engine->font;

	IoClient* connection = &this->engine->connection;

	string pts = GetWaitString();
	
	al_draw_text(font, WHITE, 20, 30, ALLEGRO_ALIGN_LEFT, "ENTER SERVER IP ADDRESS or press ");
	al_draw_text(font, WHITE, 20, 40, ALLEGRO_ALIGN_LEFT, "enter to connect to default server:");

	if(input->active){

		input->draw(30, 60);

	} else {

		int connState = connection->get_state();

		if(connState == CONNECTION_STATE_CONNECTING) {

			al_draw_textf(font, WHITE, 20, 60, ALLEGRO_ALIGN_LEFT, "Trying %s %s", server.c_str(), pts.c_str());
				
		} else if(connState == CONNECTION_STATE_DISCONNECTED) {
			
			al_draw_text(font, WHITE, 20, 60, ALLEGRO_ALIGN_LEFT, "Connection error.");
			
		} else if (connState == CONNECTION_STATE_CONNECTED) {

			al_draw_textf(font, WHITE, 20, 60, ALLEGRO_ALIGN_LEFT, "Connected to %s", server.c_str());
			al_draw_textf(font, WHITE, 20, 75, ALLEGRO_ALIGN_LEFT, "Wait please %s", pts.c_str());
		
		}

	}

}
