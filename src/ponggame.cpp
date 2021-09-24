#include "ponggame.h"

#include <cmath>
#include <stdexcept>
#include <iostream>
#include <sstream>
/*
#ifdef __ANDROID__
#include <android/log.h>
#define APPNAME "Ponggame"
#endif 
*/
#define INIX 160
#define INIY 100

using namespace std;
/*
void log(const string& txt){
	#ifdef ANDROID
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "%s", txt.c_str());
	#else 
	cout << txt;
	#endif
}
*/
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

uint32_t countR = 0;

double PongGame::random(double min, double max, bool rand_sign, bool include_max){

	double real_max = (double)mt19937::max() + (include_max ? 0 : 1);

	double rnd_factor = (double)(*this->mt)() / real_max;
	//cout << "max: " << real_max << " | rnd_factor: " << rnd_factor << endl;
	double num = min + rnd_factor * (max - min);
	
	if(rand_sign && (*this->mt)() > mt19937::max() / 2){
		num = -num;
	}
	/*
	std::stringstream logg;
	logg << "R" << countR++ << ": "<< num << endl;
	log(logg.str());
	*/
	return num;
}

int PongGame::intRandom(int min, int max, bool rand_sign){

	//int res = (int) this->random(min, max + 1, rand_sign, false);
	//cout << "[" << min << ", " << max << "] --> " << res << endl;
	

	int res = (int) random(0, max - min + 1, false, false);
	//cout << "[" << min << ", " << max << "] --> " << res << endl;
	res += min;

	if(rand_sign && (*mt)() > mt19937::max() / 2){
		res = -res;
	}
	/*
	std::stringstream logg;
	logg << "R" << countR++ << ": "<< res << endl;
	log(logg.str());
	*/
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
	
	for(auto & player: players){
		player->medlen = MEDLEN; 
		for(int j = 0; j < BONUS_MAX; j++){
			player->bonus_timers[j] = 0;
		}
	}

	ball->setParameters(INIX, INIY, tvx, 0.5 * (this->intRandom(-1, 1)));
	players[0]->setY(40);
	players[1]->setY(160);

}

void PongGame::giveScore(PlayerP* pl, int score){

	for(auto & player: players){
		if(player != pl){
			player->racha = 0;
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

void PongGame::addMessage(const std::string& evtMsg){

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
			cout << "T " << this->tick << " | P2 score: " << players[1]->score << endl;
		} else {
			this->giveScore(players[0], 1);
			cout << "T " << this->tick << " | P1 score: " << players[0]->score << endl;
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

	for(auto & pl: players){

		for(int j = 0; j < BONUS_MAX; j++){
			if(pl->bonus_timers[j] > 0){
				pl->bonus_timers[j] --;
			}
		}

	}

	for(auto & b: bonus){

		if( b->getStat() == 0 ){

			if( b->cooldown == 0 ){

				/**
				 * random generation order must be well defined so we cannot call directly 
				 * in the function call since parameter parsing order is undefined
				 */

				double p1 = this->random(100, 240);
				double p2 = this->random(70, 130);
				double p3 =	this->random(1.5, 2.0, true);
				double p4 =	this->random(1.5, 2.0, true);

				b->setParameters( p1, p2, p3, p4 );

				b->cooldown = 1000 + this->intRandom(0, 1000);

			} else {

				b->cooldown--;

			}

		}

		b->process();

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
			stat = false;
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

		if(fabs(y - players[0]->getY()) < (players[0]->medlen + radius + 2) && x > (GROSOR)){

			 this->onPlayerHit(players[0]);
			
		}

	}

	//CHOQUE CON LA PALA DCHA
	if(x >= (320 - radius - GROSOR)){

		if(fabs(y - players[1]->getY()) < (players[1]->medlen + radius + 2) && x < (320 - GROSOR)){

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

	if(p1->bonus_timers[BONUS_BALL] > 0 && p2->bonus_timers[BONUS_BALL] == 0)
		radius = (320 - x) / 20 + 2;
	else if(p2->bonus_timers[BONUS_BALL] > 0 && p1->bonus_timers[BONUS_BALL] == 0)
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

	this->game = pongGame;
	this->x = px;
	this->y = py;

	this->reset();

}

void PlayerP::reset(){

	for(int i = 0; i < CONTROL_MAX; i++){
		this->controls[i] = false;
	}

	this->medlen = MEDLEN;
	this->score = 0;
	this->racha = 0;
	this->comTxtY = -40;//DESACTIVADA
	this->comTxt = "";
	
	for(int i = 0; i < BONUS_MAX; i++){
		this->bonus_timers[i] = 0;
	}

}

void PlayerP::lockLimit(){

	if(y >= (MAX_Y - medlen - LIMIT))
		y = (MAX_Y - medlen - LIMIT - 1);
	if(y <= (medlen + LIMIT))
		y = (medlen + LIMIT + 1);

}

void PlayerP::moveIA(){

	Element *ball = this->game->ball;

	//double inc = 1 + 1 + sin((double)this->game->tick / 100.0);

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
		this->bonus_timers[BONUS_BALL] = 800;
		comTxt = "SPECIAL BALL";
		comTxtY = 150;
	}

}
