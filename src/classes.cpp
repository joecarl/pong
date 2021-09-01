#include "classes.h"
#include "utils.h"

#include <math.h>
#include <stdexcept>
#include <iostream>

#define INIX 160
#define INIY 100

using namespace std;

PongGame::PongGame(){

	ball = new Ball(this);
	players[0] = new PlayerP(1, 50);
	players[1] = new PlayerP((DEF_W - GROSOR / 2), 50);
	bonus[0] = new Bonus(this, BONUS_LONG);
	bonus[1] = new Bonus(this, BONUS_BALL);
	this->numPlayers = 0;

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
			tvx = 2 * (rand() % 3 - 1);
	}
	else tvx = (int)ball->getvX();
	players[0]->medlen = MEDLEN; 
	players[1]->medlen = MEDLEN;
	players[0]->bonus_ball = 0; players[1]->bonus_ball = 0;
	ball->setParameters(INIX, INIY, tvx, 0.5 * (rand() % 3 - 1));
	players[0]->setY(40);
	players[1]->setY(160);

	//printf("iniciado nuevo punto\n");
	
}

void PongGame::giveScore(PlayerP* pl, int score){

	for(unsigned int i = 0; i < this->numPlayers; i++){
		PlayerP* iterPl = this->players[i];
		if(iterPl != pl){
			iterPl->racha = 0;
		}
	}
	
	pl->score+= score;
	pl->racha++;

	if(pl->racha > 3){
		pl->giveBonus(BONUS_IMPA);
	}

}


void PongGame::processTick(bool* keys){

	if(this->paused || this->finished){
		return;
	}

	if(keys[ALLEGRO_KEY_G]) players[0]->medlen += 1;//DEBUG

	if(this->numPlayers == 2){
		players[1]->controlMove(keys[ALLEGRO_KEY_UP], keys[ALLEGRO_KEY_DOWN], keys[ALLEGRO_KEY_I], keys[ALLEGRO_KEY_K]);
		players[0]->controlMove(keys[ALLEGRO_KEY_W], keys[ALLEGRO_KEY_S]);
	} else {
		players[0]->controlMove(keys[ALLEGRO_KEY_UP], keys[ALLEGRO_KEY_DOWN]);
		players[1]->moveIA(this->numPlayers, ball);
	}
	
	//COMPROBAMOS QUE LA BOLA ESTÁ FUERA
	if(ball->getX() > (320 + 15) || ball->getX() < -15 ){

		if(ball->getX() < -15){
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

	ball->process(this->numPlayers, players);

	for(int i = 0; i < 2; i++){
		PlayerP* pl = this->players[i];
		if(pl->bonus_ball > 0){
			pl->bonus_ball -= 0.1;
		}
	}

	for(int i = 0; i < 2; i++){

		if( bonus[i]->getStat() == 0 && bonus_time[i] > 1000 && random_ex(0, 10000) < 1.0 ){
			bonus[i]->setParameters(random_ex(100, 240), random_ex(70, 130), random_ex(1.5, 2.0, true), random_ex(1.5, 2.0, true));
			bonus_time[i] =- 1;
		}

		bonus[i]->process(this->numPlayers, players);
		if(bonus[i]->getStat() == 0 && bonus_time[i] == -1)
			bonus_time[i] = 1;
		if(bonus_time > 0)
			bonus_time[i]++;
	}

}



//-----------------------------------------------------------------------------
//------------------------------- [ Element ] ---------------------------------

Element::Element(PongGame *game){

	this->game = game;

	vX = vY = t = 0;
	x = y = x00 = y00 = -100;

}

void Element::process(int plyrNum, PlayerP* players[]){

	if(stat){

		x = x00 + vX * t;
		y = y00 + vY * t;
		t++;

		this->preprocess();

		this->processColliding(plyrNum, players);
		
		if(x < -80 || x > DEF_W + 50){
			stat = 0;
		}

	}

}


void Element::setParameters(float px, float py, float vx, float vy, int st){

	x = x00 = px;
	y = y00 = py;
	vX = vx;
	vY = vy;
	t = 0;
	stat = st;

}

void Element::processColliding(int plyrNum, PlayerP* players[]){

	//CHOQUE CON LA PARTE DE ARRIBA
	if(y <= (radius + LIMIT))
		this->setParameters(this->x, (radius + LIMIT + 1), this->vX, -this->vY, stat);

	//CHOQUE CON LA PARTE DE ABAJO
	if(y >= (MAX_Y - radius - LIMIT))
		this->setParameters(this->x, (MAX_Y - radius - LIMIT - 1), this->vX, -this->vY, stat);

	//CHOQUE CON LA PALA IZDA
	if(x <= (radius + GROSOR)){
		if(fabs(y-players[0]->getY()) < (players[0]->medlen + radius) && x > (GROSOR)){

			 this->playerHit(players[0]);
			
		}
	}

	//CHOQUE CON LA PALA DCHA
	int grosorB;
	if(plyrNum != 0) grosorB = GROSOR;
	else grosorB = 0;

	if(x >= (320 - radius - grosorB)){

		if(fabs(y - players[1]->getY()) < (players[1]->medlen + radius) && x < (320 - grosorB)){

			this->playerHit(players[1]);
			
		}

	}

}

//-----------------------------------------------------------------------------
//--------------------------------- [ Ball ] ----------------------------------

Ball::Ball(PongGame *game): Element(game){
	
	this->radius = RADIUS;

}

void Ball::preprocess(){

	if(this->game->players[0]->bonus_ball && !this->game->players[1]->bonus_ball)
		radius = (320 - x) / 20 + 2;
	else if(this->game->players[1]->bonus_ball && !this->game->players[0]->bonus_ball)
		radius = x / 20 + 2;
	else
		radius = RADIUS;

}

void Ball::playerHit(PlayerP *pl){
	
	if(pl == this->game->players[0] || this->game->numPlayers != 0){

		if(y - pl->getY() > 2) vY += 1;
		else if (y - pl->getY() > 1) vY += 0.5;
		if(y - pl->getY() < -2) vY -= 1;
		if (y - pl->getY() < -1) vY -= 0.5;

		PlaySound(Mi, 40, 4);
		PlayAudio();

	}

	int grosorB;
	if(this->game->numPlayers != 0) grosorB = GROSOR;
	else grosorB = 0;

	float newX = pl == this->game->players[0] ? (radius + GROSOR + 1) : (320 - radius - grosorB - 1);

	this->setParameters(newX, y, -vX, vY, stat);

}


//------------------------------------------------------------------------------
//--------------------------------- [ Bonus ] ----------------------------------

Bonus::Bonus(PongGame *game, int bonus_type): Element(game){
	
	this->bonus_type = bonus_type;

	this->radius = RADIUS;//provisional

}

void Bonus::playerHit(PlayerP *pl){
	
	pl->giveBonus(this->bonus_type);
	
	this->stat = false;

}

//------------------------------------------------------------------------------

PlayerP::PlayerP(int px, int py){

	x = px;
	y = py;
	medlen = MEDLEN;
	//bonus = 0;
	score = 0;
	comTxtY = -40;//DESACTIVADA
	bonus_ball = 0;

}

void PlayerP::lockLimit(){

	if(y >= (MAX_Y - medlen - LIMIT))
		y = (MAX_Y - medlen - LIMIT - 1);
	if(y <= (medlen + LIMIT))
		y = (medlen + LIMIT + 1);

}

void PlayerP::moveIA(int plyrNum, Element *ball){

	if(plyrNum == 1){
		if(y > ball->getY()) y -= (1 + rand() % 2);
		if(y < ball->getY()) y += (1 + rand() % 2);
	}
	else if(plyrNum == 0)
		y = ball->getY();

	this->lockLimit();

}

void PlayerP::controlMove(int key_up, int key_down, int key_up2, int key_down2){

	if(key_up	|| key_up2) y -= 2;//ch='w';
	if(key_down || key_down2) y += 2;//ch='s';

	this->lockLimit();

}

void PlayerP::giveBonus(int bonus_type){

	if(bonus_type == BONUS_LONG){
		medlen += 7;
		strcpy(comTxt, "LONGERRR");
		comTxtY = 150;
	}

	else if(bonus_type == BONUS_IMPA){
		strcpy(comTxt, "UNSTOPABLE");
		comTxtY = 150;
	}

	else if(bonus_type == BONUS_BALL){
		strcpy(comTxt, "SPECIAL BALL");
		comTxtY = 150;
		bonus_ball = 80;
	}

}
