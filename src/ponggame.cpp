#include "ponggame.h"

#include <cmath>
#include <stdexcept>
#include <iostream>
#include <sstream>

#define INIX 160
#define INIY 100

using namespace std;

PongGame::PongGame(uint_fast32_t _seed) {

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

uint32_t count_r = 0;

double PongGame::random(double min, double max, bool rand_sign, bool include_max) {

	double real_max = (double)mt19937::max() + (include_max ? 0 : 1);

	double rnd_factor = (double)(*this->mt)() / real_max;
	//cout << "max: " << real_max << " | rnd_factor: " << rnd_factor << endl;
	double num = min + rnd_factor * (max - min);
	
	if (rand_sign && (*this->mt)() > mt19937::max() / 2) {
		num = -num;
	}
	/*
	std::stringstream logg;
	logg << "R" << count_r++ << ": "<< num << endl;
	log(logg.str());
	*/
	return num;
}

int PongGame::int_random(int min, int max, bool rand_sign) {

	//int res = (int) this->random(min, max + 1, rand_sign, false);
	//cout << "[" << min << ", " << max << "] --> " << res << endl;
	

	int res = (int) random(0, max - min + 1, false, false);
	//cout << "[" << min << ", " << max << "] --> " << res << endl;
	res += min;

	if (rand_sign && (*mt)() > mt19937::max() / 2) {
		res = -res;
	}
	/*
	std::stringstream logg;
	logg << "R" << count_r++ << ": "<< res << endl;
	log(logg.str());
	*/
	return res;

}

void PongGame::restart() {
	
	this->players[0]->score = 0;
	this->players[1]->score = 0;
	this->finished = false;
	this->tick = 0;

}

void PongGame::toggle_pause() {

	paused = !paused;

}

void PongGame::iniciar_punto(int first) {

	int tvx = 0;

	if (first == 1) {
		players[0]->score = 0; 
		players[1]->score = 0;
		while (tvx == 0) {
			tvx = 2 * (this->int_random(-1, 1));
		}
	} else {
		tvx = (int) ball->get_vx();
	}
	
	for (auto & player: players) {
		player->medlen = MEDLEN; 
		for (int j = 0; j < BONUS_MAX; j++) {
			player->bonus_timers[j] = 0;
		}
	}

	ball->set_parameters(INIX, INIY, tvx, 0.5 * (this->int_random(-1, 1)));
	players[0]->set_y(40);
	players[1]->set_y(160);

}

void PongGame::give_score(PlayerP* pl, int score) {

	for (auto& player: players) {
		if (player != pl) {
			player->racha = 0;
		}
	}
	
	pl->score += score;
	pl->racha++;

	if (pl->racha > 3) {
		pl->give_bonus(BONUS_IMPA);
	}

}


void PongGame::add_message(const std::string& evt_msg) {

	this->messages.push(evt_msg);

}
	
void PongGame::process_tick() {

	if (this->paused || this->finished) {
		return;
	}

	if (this->control_mode == CONTROLMODE_DEBUG) {
		
		players[0]->move_perfect();
		players[1]->move_perfect();

	} else if (this->control_mode == CONTROLMODE_TRAINING) {

		players[0]->control_move();
		players[1]->move_perfect();

	} else if (this->control_mode == CONTROLMODE_SINGLE_PLAYER) {

		players[0]->control_move();
		players[1]->move_ia();

	} else if (this->control_mode == CONTROLMODE_TWO_PLAYERS) {

		players[0]->control_move();
		players[1]->control_move();

	} else {

		//no se contempla
		
	}
	
	//COMPROBAMOS QUE LA BOLA ESTÁ FUERA
	if (ball->get_x() > (320 + 15) || ball->get_x() < -15 ) {

		if (ball->get_x() < -15) {
			this->give_score(players[1], 1);
			cout << "T " << this->tick << " | P2 score: " << players[1]->score << endl;
		} else {
			this->give_score(players[0], 1);
			cout << "T " << this->tick << " | P1 score: " << players[0]->score << endl;
		}

		this->add_message("scored");

		this->iniciar_punto(0);

	}

	#define MAX_SCORE 11
	
	if ((players[0]->score == MAX_SCORE || players[1]->score == MAX_SCORE) && this->control_mode != CONTROLMODE_TRAINING) {

		this->finished = true;
		cout << "Game finished!" << endl;
		return;

	}

	ball->process();

	for (auto & pl: players) {

		for (int j = 0; j < BONUS_MAX; j++) {
			if (pl->bonus_timers[j] > 0) {
				pl->bonus_timers[j] --;
			}
		}

	}

	for (auto & b: bonus) {

		if (b->get_stat() == 0) {

			if (b->cooldown == 0) {

				/**
				 * random generation order must be well defined so we cannot call directly 
				 * in the function call since parameter parsing order is undefined
				 */

				double p1 = this->random(100, 240);
				double p2 = this->random(70, 130);
				double p3 =	this->random(1.5, 2.0, true);
				double p4 =	this->random(1.5, 2.0, true);

				b->set_parameters(p1, p2, p3, p4);

				b->cooldown = 1000 + this->int_random(0, 1000);

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

Element::Element(PongGame *game) {

	this->game = game;

	vx = vy = t = 0;
	x = y = x00 = y00 = -100;

}

void Element::process() {

	if (stat) {

		x = x00 + vx * t;
		y = y00 + vy * t;
		t++;

		this->preprocess();

		this->process_colliding();
		
		if (x < -80 || x > DEF_W + 50) {
			stat = false;
		}

	}

}


void Element::set_parameters(double px, double py, double _vx, double _vy, int st) {

	x = x00 = px;
	y = y00 = py;
	vx = _vx;
	vy = _vy;
	t = 0;
	stat = st;

}

void Element::process_colliding() {

	 PlayerP **players = this->game->players;

	//CHOQUE CON LA PARTE DE ARRIBA
	if (y <= (radius + LIMIT)) {

		this->set_parameters(this->x, (radius + LIMIT + 1), this->vx, -this->vy, stat);

	}

	//CHOQUE CON LA PARTE DE ABAJO
	if (y >= (MAX_Y - radius - LIMIT)) {

		this->set_parameters(this->x, (MAX_Y - radius - LIMIT - 1), this->vx, -this->vy, stat);

	}

	//CHOQUE CON LA PALA IZDA
	if (x <= (radius + GROSOR)) {

		if (fabs(y - players[0]->get_y()) < (players[0]->medlen + radius + 2) && x > (GROSOR)) {

			 this->on_player_hit(players[0]);
			
		}

	}

	//CHOQUE CON LA PALA DCHA
	if (x >= (320 - radius - GROSOR)) {

		if (fabs(y - players[1]->get_y()) < (players[1]->medlen + radius + 2) && x < (320 - GROSOR)) {

			this->on_player_hit(players[1]);
			
		}

	}

}

//-----------------------------------------------------------------------------
//--------------------------------- [ Ball ] ----------------------------------

Ball::Ball(PongGame *game): Element(game) {
	
	this->radius = RADIUS;

}

void Ball::preprocess() {

	PlayerP *p1 = this->game->players[0];
	PlayerP *p2 = this->game->players[1];

	if (p1->bonus_timers[BONUS_BALL] > 0 && p2->bonus_timers[BONUS_BALL] == 0)
		radius = (320 - x) / 20 + 2;
	else if (p2->bonus_timers[BONUS_BALL] > 0 && p1->bonus_timers[BONUS_BALL] == 0)
		radius = x / 20 + 2;
	else
		radius = RADIUS;

}

void Ball::on_player_hit(PlayerP *pl) {
	
	if (pl == this->game->players[0] || this->game->control_mode != CONTROLMODE_TRAINING) {

		if (y - pl->get_y() > 2) vy += 1;
		else if (y - pl->get_y() > 1) vy += 0.5;
		if (y - pl->get_y() < -2) vy -= 1;
		if (y - pl->get_y() < -1) vy -= 0.5;

		this->game->add_message("hit");

	}

	double newX = pl == this->game->players[0] ? (radius + GROSOR + 1) : (320 - radius - GROSOR - 1);

	this->set_parameters(newX, y, -vx, vy, stat);

}


//------------------------------------------------------------------------------
//--------------------------------- [ Bonus ] ----------------------------------

Bonus::Bonus(PongGame *game, int bonus_type): Element(game) {
	
	this->bonus_type = bonus_type;

	this->radius = RADIUS;//provisional

}

void Bonus::on_player_hit(PlayerP *pl) {
	
	pl->give_bonus(this->bonus_type);
	
	this->stat = false;

}


//------------------------------------------------------------------------------
//-------------------------------- [ PlayerP ] ---------------------------------

PlayerP::PlayerP(PongGame *pong_game, int px, int py) {

	this->game = pong_game;
	this->x = px;
	this->y = py;

	this->reset();

}

void PlayerP::reset() {

	for (int i = 0; i < CONTROL_MAX; i++) {
		this->controls[i] = false;
	}

	this->medlen = MEDLEN;
	this->score = 0;
	this->racha = 0;
	this->com_txt_y = -40;//DESACTIVADA
	this->com_txt = "";
	
	for (int i = 0; i < BONUS_MAX; i++) {
		this->bonus_timers[i] = 0;
	}

}

void PlayerP::lock_limit() {

	if (y >= (MAX_Y - medlen - LIMIT))
		y = MAX_Y - medlen - LIMIT - 1;
	if (y <= (medlen + LIMIT))
		y = medlen + LIMIT + 1;

}

void PlayerP::move_ia() {

	Element *ball = this->game->ball;

	//double inc = 1 + 1 + sin((double)this->game->tick / 100.0);

	if (y > ball->get_y()) y -= (1 + this->game->int_random(0, 1));
	if (y < ball->get_y()) y += (1 + this->game->int_random(0, 1));
	
	this->lock_limit();

}

void PlayerP::move_perfect() {

	y = this->game->ball->get_y();
	
	this->lock_limit();

}

void PlayerP::control_move() {

	if (controls[CONTROL_MOVE_UP]) y -= 2;
	if (controls[CONTROL_MOVE_DOWN]) y += 2;

	this->lock_limit();

}

void PlayerP::give_bonus(int bonus_type) {

	if (bonus_type == BONUS_LONG) {
		medlen += 7;
		com_txt = "LONGERRR";
		com_txt_y = 150;
	}

	else if (bonus_type == BONUS_IMPA) {
		com_txt = "UNSTOPABLE";
		com_txt_y = 150;
	}

	else if (bonus_type == BONUS_BALL) {
		this->bonus_timers[BONUS_BALL] = 800;
		com_txt = "SPECIAL BALL";
		com_txt_y = 150;
	}

}
