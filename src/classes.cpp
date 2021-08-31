#include "classes.h"
#include "utils.h"

#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <math.h>
#include <stdexcept>
#include <iostream>

#define INIX 160
#define INIY 100

using namespace std;

PongGame::PongGame(float sc, ALLEGRO_FONT* font){

	ball = new element(true, 0, "");
	players[0] = new playerP(scale * 1, 50, font);
	players[1] = new playerP(scale * (DEF_W-GROSOR/2), 50, font);
	bonus[0] = new element(false, BONUS_LONG, LONG_DIR);
	bonus[1] = new element(false, BONUS_BALL, BALL_DIR);
	this->numPlayers = 0;
	this->scale = sc;

}

void PongGame::restart(){
	
	this->players[0]->score = 0;
	this->players[1]->score = 0;
	this->finished = false;

}

void PongGame::togglePause(){

	paused = !paused;

}

void PongGame::iniciarPunto(int first){

	int tvx = 0;
	if(first == 1){
		players[0]->score = 0; 
		players[1]->score = 0;
		while(tvx == 0)
			tvx = scale * 2 * (rand() % 3 - 1);
	}
	else tvx = (int)ball->GetvX();
	players[0]->medlen = MEDLEN; 
	players[1]->medlen = MEDLEN;
	players[0]->bonus_ball = 0; players[1]->bonus_ball = 0;
	ball->SetParameters(scale * INIX, scale * INIY, tvx, scale * 0.5 * (rand() % 3 - 1));
	players[0]->SetY(scale * 40);
	players[1]->SetY(scale * 160);

	//printf("iniciado nuevo punto\n");
	
}

void PongGame::giveScore(playerP* pl, int score){

	for(unsigned int i = 0; i < this->numPlayers; i++){
		playerP* iterPl = this->players[i];
		if(iterPl != pl){
			iterPl->racha = 0;
		}
	}
	
	pl->score+= score;
	pl->racha++;

	if(pl->racha > 3){
		pl->GiveBonus(BONUS_IMPA);
	}

}


void PongGame::processTick(bool* keys){

	if(this->paused || this->finished){
		return;
	}

	if(keys[ALLEGRO_KEY_G]) players[0]->medlen += 1;//DEBUG

	if(this->numPlayers == 2){
		players[1]->ControlMove(scale, keys[ALLEGRO_KEY_UP], keys[ALLEGRO_KEY_DOWN], keys[ALLEGRO_KEY_I], keys[ALLEGRO_KEY_K]);
		players[0]->ControlMove(scale, keys[ALLEGRO_KEY_W], keys[ALLEGRO_KEY_S]);
	} else {
		players[0]->ControlMove(scale, keys[ALLEGRO_KEY_UP], keys[ALLEGRO_KEY_DOWN]);
		players[1]->MoveIA(this->numPlayers, ball, scale);
	}
	
	//COMPROBAMOS QUE LA BOLA ESTÁ FUERA
	if(ball->GetX() > scale * (320 + 15) || ball->GetX() < scale * (-15) ){

		if(ball->GetX() < -15){
			this->giveScore(players[1], 1);
		} else {
			this->giveScore(players[0], 1);
		}

		//this->triggerEvent("scored");

		PlaySound(Re, 130);
		PlaySound(Do, 250);
		PlayAudio();

		this->iniciarPunto(0);

	}
	
	if((players[0]->score == 11 || players[1]->score == 11) && this->numPlayers != 0){

		this->finished = true;
		return;

	}

	ball->Process(scale, this->numPlayers, players);

	for(int i = 0; i < 2; i++){

		if( bonus[i]->GetStat() == 0 && bonus_time[i] > 1000 && random_ex(0, 10000) < 1.0 ){
			bonus[i]->SetParameters(random_ex(100, 240), random_ex(70, 130), random_ex(1.5, 2.0, true), random_ex(1.5, 2.0, true));
			bonus_time[i] =- 1;
		}

		bonus[i]->Process(scale, this->numPlayers, players);
		if(bonus[i]->GetStat() == 0 && bonus_time[i] == -1)
			bonus_time[i] = 1;
		if(bonus_time > 0)
			bonus_time[i]++;
	}

}









element::element(bool iscircle, int bonus, const char *filename){

	vX = vY = t = 0;
	x = y = x00 = y00 = -100;

	if(!iscircle){
		cout << "Loading bitmap: " << filename << endl;
		sprite = al_load_bitmap(filename);
		al_convert_mask_to_alpha(sprite, ALPHA_COLOR);
		if(!sprite){
			throw std::runtime_error("error loading bitmap");
		}
	}

	isCircle = iscircle;
	bonus_type = bonus;
	radius = RADIUS;

}

void element::Process(int scale, int plyrNum, playerP* players[]){

	if(stat){

		x = x00 + vX * t;
		y = y00 + vY * t;
		t++;

		if(isCircle){
			if(players[0]->bonus_ball && !players[1]->bonus_ball)
				radius = (scale * 320 - x) / 20 + 2;
			else if(players[1]->bonus_ball && !players[0]->bonus_ball)
				radius = x / 20 + 2;
			else
				radius= RADIUS;
		}

		ProcessColliding(scale, plyrNum, players);
		
		if(x < -scale * 80 || x > scale * DEF_W + 50){
			stat = 0;
		}

	}

}

void element::Draw(int scale){

	if(!this->stat){
		return;
	}

	if(isCircle){

		al_draw_filled_circle(x, y, radius*scale, al_map_rgb( 255,255, 255));

	} else {

		al_draw_bitmap(sprite, x - 10, y - 10, 0);// x-sprite->w/2, y-sprite->h/2);

	}

}

void element::SetParameters(float px, float py, float vx, float vy, int st){

	x = x00 = px;
	y = y00 = py;
	vX = vx;
	vY = vy;
	t = 0;
	stat = st;

}

void element::ProcessColliding(int scale, int plyrNum, playerP* players[]){

	//CHOQUE CON LA PARTE DE ARRIBA
	if(y <= scale * (radius + LIMIT))
		SetParameters(this->x, scale*(radius+LIMIT+1), this->vX, -this->vY, stat);

	//CHOQUE CON LA PARTE DE ABAJO
	if(y >= scale * (MAX_Y - radius - LIMIT))
		SetParameters(this->x, scale * (MAX_Y - radius - LIMIT - 1), this->vX, -this->vY, stat);

	//CHOQUE CON LA PALA IZDA
	if(x <= scale * (radius + GROSOR)){
		if(fabs(y-players[0]->GetY()) < scale * (players[0]->medlen+radius) && x > scale * (GROSOR)){
			if(isCircle){
				if(y - players[0]->GetY() > scale*2) vY += 1;
				else if (y - players[0]->GetY() > scale*1) vY += 0.5;
				if(y - players[0]->GetY() < -scale*2) vY -= 1;
				if (y - players[0]->GetY() < -scale*1) vY -= 0.5;
					PlaySound(Mi, 40, 4);
					PlayAudio();
				SetParameters(scale * (radius + GROSOR + 1), y, -vX, vY, stat);
			} else {
				stat = 0;
				players[0]->GiveBonus(bonus_type);
			}
		}
	}

	//CHOQUE CON LA PALA DCHA
	int grosorB;
	if(plyrNum != 0) grosorB = GROSOR;
	else grosorB = 0;

	if(x >= scale * (320 - radius - grosorB)){

		if(fabs(y - players[1]->GetY()) < scale * (players[1]->medlen + radius) && x < scale * (320 - grosorB)){

			if(isCircle){

				if(plyrNum != 0){
					if(y - players[1]->GetY() > scale * 2) vY += 1;
					else if (y - players[1]->GetY() > scale * 1) vY += 0.5;
					if(y - players[1]->GetY() < -scale * 2) vY -= 1;
					if (y - players[1]->GetY() < -scale * 1) vY -= 0.5;
					PlaySound(Mi, 40, 4);
					PlayAudio();
				}
				SetParameters(scale*(320 - radius - grosorB - 1), y, -vX, vY, stat);

			} else {

				stat = 0;
				players[1]->GiveBonus(bonus_type);
			}

		}

	}

}

//------------------------------------------------------------------------------

playerP::playerP(int px, int py, ALLEGRO_FONT* _font){

	font = _font;
	x = px;
	y = py;
	medlen = MEDLEN;
	//bonus = 0;
	score = 0;
	comTxtY = -40;//DESACTIVADA
	bonus_ball = 0;

}

void playerP::LockLimit(int scale){

	if(y >= scale * (MAX_Y - medlen - LIMIT))
		y = scale * (MAX_Y - medlen - LIMIT - 1);
	if(y <= scale * (medlen + LIMIT))
		y = scale * (medlen + LIMIT + 1);

}

void playerP::Draw(int scale){

	int medln = this->medlen;
	
	al_draw_filled_rectangle(
		x - 2, 
		y - scale * medln, 
		x + 2, 
		y + scale * medln, 
		WHITE
	);
	al_draw_filled_rectangle(
		x - 1,
		y - scale * medln - 1, 
		x + 1, 
		y + scale * medln+1, 
		WHITE
	);
	 
	static int bonus_ball_decrease = 10;

	if(bonus_ball){
		if(x < 100)	al_draw_filled_rectangle(0, 16, 158.0 * bonus_ball / 80.0, 20, al_map_rgb(0, 200, 50));
		else		al_draw_filled_rectangle(DEF_W, 16, DEF_W - 158.0 * bonus_ball / 80.0, 20, al_map_rgb(0, 200, 50));
		
		bonus_ball_decrease--;
		if (bonus_ball_decrease <= 0){
			bonus_ball_decrease = 10;
			bonus_ball--;
		}
	}

	if(comTxtY >- 40){
		int txtX;
		if(x<100) txtX = scale * 60;
		else txtX = scale * 220;
		al_draw_text(font, al_map_rgb( 150-comTxtY,150-comTxtY*4, 255), txtX, comTxtY, ALLEGRO_ALIGN_CENTER, comTxt);//al_map_rgb(255,0,255) );
		comTxtY -= 2;
	}

}

void playerP::MoveIA(int plyrNum, element *ball,int scale){

	if(plyrNum == 1){
		if(y > ball->GetY()) y -= scale * (1 + rand() % 2);
		if(y < ball->GetY()) y += scale * (1 + rand() % 2);
	}
	else if(plyrNum == 0)
		y = ball->GetY();

	LockLimit(scale);

}

void playerP::ControlMove(int scale, int key_up, int key_down,int key_up2, int key_down2){

	if(key_up	|| key_up2) y -= scale*2;//ch='w';
	if(key_down || key_down2) y += scale*2;//ch='s';
	LockLimit(scale);

}

void playerP::GiveBonus(int bonus_type){

	if(bonus_type == BONUS_LONG){
		medlen += 7;
		strcpy(comTxt, "LONGERRR");
		comTxtY = 150;
	}

	if(bonus_type == BONUS_IMPA){
		strcpy(comTxt, "UNSTOPABLE");
		comTxtY = 150;
	}

	if(bonus_type == BONUS_BALL){
		strcpy(comTxt, "SPECIAL BALL");
		comTxtY = 150;
		bonus_ball = 80;
	}

}
