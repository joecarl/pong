#include "ponggame.h"

#include <math.h>
#include <stdexcept>
#include <iostream>

#define INIX 160
#define INIY 100

using namespace std;

PongGame::PongGame(uint_fast32_t _seed){

	this->seed = _seed;

	this->mt = new mt19937(_seed);

	//cout << "TEST:" << (*this->mt)() << endl;

	ball = new Ball(this);
	players[0] = new PlayerP(this, 1, 50);
	players[1] = new PlayerP(this, (DEF_W - GROSOR / 2), 50);
	bonus[0] = new Bonus(this, BONUS_LONG);
	bonus[1] = new Bonus(this, BONUS_BALL);

	//cout << "TEST:" << (*this->mt)() << endl;

}

double PongGame::random(double min, double max, bool rand_sign, bool include_max){

	double real_max = (double)this->mt->max() + (include_max ? 0 : 1);

	double rnd_factor = (double)(*this->mt)() / real_max;
	//cout << "max: " << real_max << " | rnd_factor: " << rnd_factor << endl;
	double num = min + rnd_factor * (max - min);
	
	if(rand_sign && (*this->mt)() > this->mt->max() / 2){
		num = -num;
	}

	return num;
}

int PongGame::intRandom(int min, int max, bool rand_sign){

	int res = (int) this->random(min, max + 1, rand_sign, false);
	//cout << "[" << min << ", " << max << "] --> " << res << endl;
	return res;

}

void PongGame::restart(){
	
	this->players[0]->score = 0;
	this->players[1]->score = 0;
	this->finished = false;
	this->tick = 0;

}

void PongGame::togglePause(){

	paused = !paused;

}

void PongGame::iniciarPunto(int first){

	int tvx = 0;

	if(first == 1){
		players[0]->score = 0; 
		players[1]->score = 0;
		while(tvx == 0){
			tvx = 2 * (this->intRandom(-1, 1));
		}
	} else {
		tvx = (int)ball->getvX();
	}
	
	players[0]->medlen = MEDLEN; 
	players[1]->medlen = MEDLEN;
	players[0]->bonus_ball = 0; players[1]->bonus_ball = 0;
	ball->setParameters(INIX, INIY, tvx, 0.5 * (this->intRandom(-1, 1)));
	players[0]->setY(40);
	players[1]->setY(160);

}

void PongGame::giveScore(PlayerP* pl, int score){

	for(unsigned int i = 0; i < 2; i++){
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

/*
void PongGame::addEventListener(string &evtName, function<void>& fn){

	this->eventListeners.push_back(
		EvtListener({evtName, fn})
	);

}
*/

void PongGame::addMessage(std::string evtMsg){

	this->messages.push(evtMsg);

}
	
void PongGame::processTick(){

	if(this->paused || this->finished){
		return;
	}

	if(this->controlMode == CONTROLMODE_DEBUG){
		
		players[0]->movePerfect();
		players[1]->movePerfect();

	} else if(this->controlMode == CONTROLMODE_TRAINING){

		players[0]->controlMove();
		players[1]->movePerfect();

	} else if(this->controlMode == CONTROLMODE_SINGLE_PLAYER){

		players[0]->controlMove();
		players[1]->moveIA();

	} else if(this->controlMode == CONTROLMODE_TWO_PLAYERS){

		players[0]->controlMove();
		players[1]->controlMove();

	} else {

		//no se contempla
		
	}
	
	//COMPROBAMOS QUE LA BOLA ESTÁ FUERA
	if(ball->getX() > (320 + 15) || ball->getX() < -15 ){

		if(ball->getX() < -15){
			this->giveScore(players[1], 1);
		} else {
			this->giveScore(players[0], 1);
		}

		this->addMessage("scored");

		this->iniciarPunto(0);

	}
	
	if((players[0]->score == 11 || players[1]->score == 11) && this->controlMode != CONTROLMODE_TRAINING){

		this->finished = true;
		cout << "Game finished!" << endl;
		return;

	}

	ball->process();

	for(int i = 0; i < 2; i++){
		PlayerP* pl = this->players[i];
		if(pl->bonus_ball > 0){
			pl->bonus_ball -= 0.1;
		}
	}

	for(int i = 0; i < 2; i++){

		if( bonus[i]->getStat() == 0 && bonus_time[i] > 1000 && this->intRandom(0, 100) == 0 ){
			bonus[i]->setParameters(
				this->random(100, 240), 
				this->random(70, 130), 
				this->random(1.5, 2.0, true), 
				this->random(1.5, 2.0, true)
			);
			bonus_time[i] =- 1;
		}

		bonus[i]->process();
		if(bonus[i]->getStat() == 0 && bonus_time[i] == -1)
			bonus_time[i] = 1;
		if(bonus_time > 0)
			bonus_time[i]++;
	}

	this->tick++;
}



//-----------------------------------------------------------------------------
//------------------------------- [ Element ] ---------------------------------

Element::Element(PongGame *game){

	this->game = game;

	vX = vY = t = 0;
	x = y = x00 = y00 = -100;

}

void Element::process(){

	if(stat){

		x = x00 + vX * t;
		y = y00 + vY * t;
		t++;

		this->preprocess();

		this->processColliding();
		
		if(x < -80 || x > DEF_W + 50){
			stat = 0;
		}

	}

}


void Element::setParameters(double px, double py, double vx, double vy, int st){

	x = x00 = px;
	y = y00 = py;
	vX = vx;
	vY = vy;
	t = 0;
	stat = st;

}

void Element::processColliding(){

	 PlayerP **players = this->game->players;

	//CHOQUE CON LA PARTE DE ARRIBA
	if(y <= (radius + LIMIT)){

		this->setParameters(this->x, (radius + LIMIT + 1), this->vX, -this->vY, stat);

	}

	//CHOQUE CON LA PARTE DE ABAJO
	if(y >= (MAX_Y - radius - LIMIT)){

		this->setParameters(this->x, (MAX_Y - radius - LIMIT - 1), this->vX, -this->vY, stat);

	}

	//CHOQUE CON LA PALA IZDA
	if(x <= (radius + GROSOR)){

		if(fabs(y-players[0]->getY()) < (players[0]->medlen + radius) && x > (GROSOR)){

			 this->onPlayerHit(players[0]);
			
		}

	}

	//CHOQUE CON LA PALA DCHA
	
	if(x >= (320 - radius - GROSOR)){

		if(fabs(y - players[1]->getY()) < (players[1]->medlen + radius) && x < (320 - GROSOR)){

			this->onPlayerHit(players[1]);
			
		}

	}

}

//-----------------------------------------------------------------------------
//--------------------------------- [ Ball ] ----------------------------------

Ball::Ball(PongGame *game): Element(game){
	
	this->radius = RADIUS;

}

void Ball::preprocess(){

	PlayerP *p1 = this->game->players[0];
	PlayerP *p2 = this->game->players[1];

	if(p1->bonus_ball > 0 && p2->bonus_ball <= 0)
		radius = (320 - x) / 20 + 2;
	else if(p2->bonus_ball > 0 && p1->bonus_ball <= 0)
		radius = x / 20 + 2;
	else
		radius = RADIUS;

}

void Ball::onPlayerHit(PlayerP *pl){
	
	if(pl == this->game->players[0] || this->game->controlMode != CONTROLMODE_TRAINING){

		if(y - pl->getY() > 2) vY += 1;
		else if (y - pl->getY() > 1) vY += 0.5;
		if(y - pl->getY() < -2) vY -= 1;
		if (y - pl->getY() < -1) vY -= 0.5;

		this->game->addMessage("hit");

	}

	double newX = pl == this->game->players[0] ? (radius + GROSOR + 1) : (320 - radius - GROSOR - 1);

	this->setParameters(newX, y, -vX, vY, stat);

}


//------------------------------------------------------------------------------
//--------------------------------- [ Bonus ] ----------------------------------

Bonus::Bonus(PongGame *game, int bonus_type): Element(game){
	
	this->bonus_type = bonus_type;

	this->radius = RADIUS;//provisional

}

void Bonus::onPlayerHit(PlayerP *pl){
	
	pl->giveBonus(this->bonus_type);
	
	this->stat = false;

}


//------------------------------------------------------------------------------
//-------------------------------- [ PlayerP ] ---------------------------------

PlayerP::PlayerP(PongGame *pongGame, int px, int py){

	for(int i = 0; i < CONTROL_MAX; i++){
		this->controls[i] = false;
	}
	
	this->game = pongGame;
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

void PlayerP::moveIA(){

	Element *ball = this->game->ball;

	if(y > ball->getY()) y -= (1 + this->game->intRandom(0, 1));
	if(y < ball->getY()) y += (1 + this->game->intRandom(0, 1));
	
	this->lockLimit();

}

void PlayerP::movePerfect(){

	y = this->game->ball->getY();
	
	this->lockLimit();

}

void PlayerP::controlMove(){

	if(controls[CONTROL_MOVE_UP]) y -= 2;
	if(controls[CONTROL_MOVE_DOWN]) y += 2;

	this->lockLimit();

}

void PlayerP::giveBonus(int bonus_type){

	if(bonus_type == BONUS_LONG){
		medlen += 7;
		comTxt = "LONGERRR";
		comTxtY = 150;
	}

	else if(bonus_type == BONUS_IMPA){
		comTxt = "UNSTOPABLE";
		comTxtY = 150;
	}

	else if(bonus_type == BONUS_BALL){
		bonus_ball = 80;
		comTxt = "SPECIAL BALL";
		comTxtY = 150;
	}

}
