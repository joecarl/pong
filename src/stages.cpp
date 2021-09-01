#include <iostream>
#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>

#include <string>
#include <math.h>

#include "classes.h"
#include "utils.h"
#include "hengine.h"
#include "stages.h"

#define ALPHA_COLOR al_map_rgb(255,0,255)


using namespace std;


string GetData(string pkg, string field){

	size_t pos = pkg.find(field);
	if(pos == std::string::npos)
		return "";
	pkg = &pkg[pos + field.length() + 1];
	pos = pkg.find(" ");
	if(pos != std::string::npos)
		pkg.resize(pos);
	return pkg;

}

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

	PongGame* pongGame = this->engine->pongGame;

	if(event.type == ALLEGRO_EVENT_KEY_DOWN){
	
		//isClient = false;
		////al_draw_textf(font,WHITE,20, 75, ALLEGRO_ALIGN_LEFT,"%f", sin(x)*sin(x));
		
		if(keycode == ALLEGRO_KEY_1) pongGame->numPlayers = 1;
		if(keycode == ALLEGRO_KEY_2) pongGame->numPlayers = 2;
		if(keycode == ALLEGRO_KEY_3) pongGame->numPlayers = 0;
		if(keycode == ALLEGRO_KEY_4){
			pongGame->numPlayers = 2;
			//isClient = true;
			this->engine->setStage(CONN); 
		}
		
		if(keycode == ALLEGRO_KEY_C){//CAMBIAR RESOLUCION, bad performance
			/*
			scale = scale == 1 ? 2 : 1;
			al_destroy_display(display);
			al_destroy_font(font);
			player[1]->SetX(scale * (DEF_W - GROSOR / 2));
			font = al_load_ttf_font("resources/font.ttf", scale * 9, 0) ;
			buffer = al_create_bitmap(scale * resX, scale * resY);
			*/
		}

		if(keycode == ALLEGRO_KEY_ESCAPE){
			this->engine->finish = true;
		}

		else if(keycode == ALLEGRO_KEY_1 || keycode == ALLEGRO_KEY_2 || keycode == ALLEGRO_KEY_3) {

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

	if(this->engine->pongGame->numPlayers == 0){
		
		al_draw_line(scale*DEF_W, minCourtY, scale*DEF_W, maxCourtY, al_map_rgb( 255, 255, 255), 2);

	}

}

void GameStage::drawScores(){
	
	PongGame* pongGame = this->engine->pongGame;
	ALLEGRO_FONT* font = this->engine->font;
	float scale = this->engine->scale;

	if(pongGame->numPlayers != 0){
		al_draw_textf( font, al_map_rgb( 255, 0, 0), scale*25, scale*186, ALLEGRO_ALIGN_LEFT, "SCORE:%d", pongGame->players[0]->score );
		al_draw_textf( font, al_map_rgb( 255, 0, 0), scale*240, scale*186, ALLEGRO_ALIGN_LEFT, "SCORE:%d", pongGame->players[1]->score );
	} else {
		al_draw_textf( font, al_map_rgb( 255, 0, 0), scale*25, scale*186, ALLEGRO_ALIGN_LEFT, "FAILS:%d", pongGame->players[1]->score );
	}

}

void GameStage::onEnterStage(){

	PongGame* pongGame = this->engine->pongGame;
		
	PlaySound(Do, 400);
	PlaySound(Re, 200);
	PlaySound(La, 100);
	PlaySound(Si, 100);
	PlayAudio();

	pongGame->restart();
	pongGame->iniciarPunto(1);

	delayer = 75;

}

void GameStage::onEvent(ALLEGRO_EVENT evt){

	PongGame* pongGame = this->engine->pongGame;

	int kCode = evt.keyboard.keycode;
	
	if(evt.type == ALLEGRO_EVENT_KEY_DOWN){

		if(kCode == ALLEGRO_KEY_ESCAPE){

			this->engine->setStage(MENU);

		}

		else if(kCode == ALLEGRO_KEY_P){//P (PAUSA)
			
			this->engine->pongGame->togglePause();

		}

	}

	if(evt.type == ALLEGRO_EVENT_KEY_DOWN || evt.type == ALLEGRO_EVENT_KEY_UP){

		bool newSt = evt.type == ALLEGRO_EVENT_KEY_DOWN;

		if(pongGame->numPlayers == 2){
			
			if (kCode == ALLEGRO_KEY_UP || kCode == ALLEGRO_KEY_I)
				pongGame->players[1]->controls[CONTROL_MOVE_UP] = newSt;

			else if (kCode == ALLEGRO_KEY_DOWN || kCode == ALLEGRO_KEY_K)
				pongGame->players[1]->controls[CONTROL_MOVE_DOWN] = newSt;
				
			else if (kCode == ALLEGRO_KEY_W)
				pongGame->players[0]->controls[CONTROL_MOVE_UP] = newSt;
				
			else if (kCode == ALLEGRO_KEY_S)
				pongGame->players[0]->controls[CONTROL_MOVE_DOWN] = newSt;

		} else {

			if (kCode == ALLEGRO_KEY_UP)
				pongGame->players[0]->controls[CONTROL_MOVE_UP] = newSt;
				
			else if (kCode == ALLEGRO_KEY_DOWN)
				pongGame->players[0]->controls[CONTROL_MOVE_DOWN] = newSt;

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

	PongGame* pongGame = this->engine->pongGame;

	if (delayer > 0){
		delayer--;
		return;
	} 

	if(pongGame->paused) {
		return;
	}

	//if(keys[ALLEGRO_KEY_G]) players[0]->medlen += 1;//DEBUG


	pongGame->processTick();//this->engine->keys);

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

	PongGame* pongGame = this->engine->pongGame;
	
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

		//NETWORK
		//static int counter, time;
		bool isClient = false;//, sendping;

		if(isClient){
			/*
			int	pingCounter = 1;
			int pingms;
			if ((counter++) % 60 == 1){// && sendping){
				time = milliseconds;
				sendping = false;
				connection.SendPacket("PING:" + boost::to_string(pingCounter++) + " \r\n");
			}

			//stringstream buff;
			//connection.SendPacket(buff.str());
			
			while (const char* data=connection.FetchPacket()) {
				if (GetData(data, "PING") == boost::to_string(pingCounter - 1)) {
					pingms = milliseconds - time;
					sendping = true;
				}
				if(GetData(data, "PING") == ""){
					string pkg=data;
					player[0]->SetY(atoi(GetData(pkg, "P1Y").c_str()));
					player[1]->SetY(atoi(GetData(pkg, "P2Y").c_str()));
					player[0]->score=atoi(GetData(pkg, "S1").c_str());
					player[1]->score=atoi(GetData(pkg, "S2").c_str());
					//Ball->SetPos(atoi(GetData(pkg, "BX").c_str()), atoi(GetData(pkg, "BY").c_str()));
					
					if(GetData(pkg, "W") == "1" || GetData(pkg, "W") == "2"){
						gScreen = OVER;
						
					}
				}
			}

			
			al_draw_textf(font, al_map_rgb(255, 0, 0), scale * resX / 2, scale * 2, ALLEGRO_ALIGN_CENTER, "PING:%d", pingms);
			//[END]NETWORK
			*/
		} 

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

			al_draw_text(font, al_map_rgb(255, 0, 0), scale * 320 / 2, scale * 2, ALLEGRO_ALIGN_CENTER, "Press ESC to Main Menu");
				
			this->drawCourt();

			Tracer *tr = this->tracer;

			tr->drawBall(pongGame->ball, scale);
			tr->drawBonus(pongGame->bonus[0], scale);
			tr->drawBonus(pongGame->bonus[1], scale);

			this->drawScores();

			al_draw_textf(font, al_map_rgb(255, 0, 0), scale * 160, scale * 186, ALLEGRO_ALIGN_CENTER, "FPS: %d", (int)(this->engine->fps));

			tr->drawPlayer(pongGame->players[0], scale);
			if(pongGame->numPlayers != 0){
				tr->drawPlayer(pongGame->players[1], scale);
			}

		}

	}

}


ALLEGRO_BITMAP* load_bitmap(string filename){

	cout << "Loading bitmap: " << filename << endl;

	ALLEGRO_BITMAP* sprite = al_load_bitmap(filename.c_str());
	al_convert_mask_to_alpha(sprite, ALPHA_COLOR);

	if(!sprite){
		throw std::runtime_error("error loading bitmap");
	}

	return sprite;

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

}

void GameOverStage::onEvent(ALLEGRO_EVENT event){

	if(event.type == ALLEGRO_EVENT_KEY_DOWN){

		int keycode = event.keyboard.keycode;

		if(keycode == ALLEGRO_KEY_Y){
			this->engine->setStage(GAME);
		} 
		else if(keycode == ALLEGRO_KEY_N){
			this->engine->setStage(MENU);
		}

	}

}

void GameOverStage::draw(){

	PongGame* pongGame = this->engine->pongGame;
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
	connection.Reset();
	
}

void ConnStage::onEvent(ALLEGRO_EVENT event){

	if(event.type == ALLEGRO_EVENT_KEY_CHAR){

		if(input->active){
			
			if (event.keyboard.keycode != ALLEGRO_KEY_ENTER) {
				input->processKey(event.keyboard.unichar, event.keyboard.keycode);
			} else {
				input->finish();
				server = input->getValue();
				start_connection = true;
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

void ConnStage::draw(){

	ALLEGRO_FONT* font = this->engine->font;

	string pts = GetWaitString();
	
	al_draw_text(font, WHITE, 20, 30, ALLEGRO_ALIGN_LEFT, "ENTER SERVER IP ADDRESS or press ");
	al_draw_text(font, WHITE, 20, 40, ALLEGRO_ALIGN_LEFT, "enter to connect to default server:");

	if (!input->active && !connection.connected) {
		if(!connection.stopped){
			al_draw_textf(font, WHITE, 20, 60, ALLEGRO_ALIGN_LEFT, "Trying %s %s", server.c_str(), pts.c_str());
			if (start_connection) {
				
				if (server == ""){
					server = "copinstar.com";
				}
				connection.Connect(server, "25000");//addr
				start_connection = false;
				
			}
		} else {
			al_draw_text(font, WHITE, 20, 60, ALLEGRO_ALIGN_LEFT, "Connection error.");
		}
		
	} else if (connection.connected && !connection.stopped) {

		al_draw_textf(font, WHITE, 20, 60, ALLEGRO_ALIGN_LEFT, "Connected to %s", server.c_str());
		al_draw_textf(font, WHITE, 20, 75, ALLEGRO_ALIGN_LEFT, "Wait please %s", pts.c_str());
		
		static int delay = 0;
		if((delay++) % 40 == 20){
			connection.SendPacket("PING:-1 \r\n");
			if(const char* pkg = connection.FetchPacket()){
				string pk = pkg;
				if(GetData(pkg, "PING") != ""){
					this->engine->setStage(GAME);
					//isClient = 1;
					cout << "Conexion OK!" << endl;
					//sendping = true;
				}
			}
		}
	}

	if(input->active){
		input->draw(30, 60);
	}
}
