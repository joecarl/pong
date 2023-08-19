#include "ponggame.h"

#include <cmath>
#include <stdexcept>
#include <iostream>
#include <sstream>

#define INIX 160
#define INIY 100

using namespace std;


PongGame::PongGame(uint_fast32_t _seed) :
	rnd(_seed)
{

	ball = new Ball(this);
	players[0] = new PlayerP(this, 1, 50);
	players[1] = new PlayerP(this, (DEF_W - GROSOR / 2), 50);
	bonus[0] = new Bonus(this, BONUS_LONG);
	bonus[1] = new Bonus(this, BONUS_BALL);
	bonus[2] = new Bonus(this, BONUS_INVI);

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

	this->warmup = 40;

	int tvx = 0;

	if (first == 1) {
		players[0]->score = 0; 
		players[1]->score = 0;
		tvx = INITIAL_SPEED * this->rnd.int_random(1, 1, true);
	} else {
		tvx = ball->get_vx() < 0 ? -INITIAL_SPEED : INITIAL_SPEED;
	}
	
	for (auto & player: players) {
		player->medlen = MEDLEN; 
		for (int j = 0; j < BONUS_MAX; j++) {
			player->bonus_timers[j] = 0;
		}
	}

	ball->set_parameters(INIX, INIY, tvx, 0.5 * (this->rnd.int_random(-1, 1)));
	ball->radius = INITIAL_RADIUS;
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
		pl->set_com_txt("UNSTOPABLE");
	}

}


void PongGame::add_message(const std::string& evt_msg) {

	this->messages.push(evt_msg);

}

uint8_t PongGame::get_winner() {

	if (!this->finished) {
		return 0;
	}

	if (this->players[0]->score > this->players[1]->score) return 1;
	else return 2;

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
	
	if ((players[0]->score == MAX_SCORE || players[1]->score == MAX_SCORE) && this->control_mode != CONTROLMODE_TRAINING) {

		this->finished = true;
		cout << "Game finished!" << endl;
		return;

	}

	if (this->warmup > 0) {

		this->warmup--;

	} else {

		ball->process();
		
	}

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

				auto& rnd = this->rnd;

				double p1 = rnd.random(100, 240);
				double p2 = rnd.random(70, 130);
				double p3 =	rnd.random(1.5, 2.0, true);
				double p4 =	rnd.random(1.5, 2.0, true);

				b->set_parameters(p1, p2, p3, p4);

				b->cooldown = 1400 + rnd.int_random(0, 1400);

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

	vx = vy = 0;
	x = y = -100;

}

void Element::process() {

	if (stat) {

		const float inct = 1.0;
		x += vx * inct;
		y += vy * inct;
	
		this->preprocess();

		this->process_colliding();
		
		if (x < -80 || x > DEF_W + 50) {
			stat = false;
		}

	}

}


void Element::set_parameters(double px, double py, double _vx, double _vy, int st) {

	x = px;
	y = py;
	vx = _vx;
	vy = _vy;
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
	
	this->radius = INITIAL_RADIUS;

}

void Ball::preprocess() {

	PlayerP *p1 = this->game->players[0];
	PlayerP *p2 = this->game->players[1];

	if (p1->bonus_timers[BONUS_BALL] > 0 && p2->bonus_timers[BONUS_BALL] == 0)
		radius = (320 - x) / 20 + 2;
	else if (p2->bonus_timers[BONUS_BALL] > 0 && p1->bonus_timers[BONUS_BALL] == 0)
		radius = x / 20 + 2;
	else
		radius = INITIAL_RADIUS;

	this->calc_invisiball_state();

}


void Ball::calc_invisiball_state() {

	auto b = this;
	auto& p1_bonus_invi_tm = game->players[0]->bonus_timers[BONUS_INVI];
	auto& p2_bonus_invi_tm = game->players[1]->bonus_timers[BONUS_INVI];
	
	const bool check_invisi_ball = 
		(p1_bonus_invi_tm > 0 && this->vx > 0) ||
		(p2_bonus_invi_tm > 0 && this->vx < 0);

	if (!check_invisi_ball) {
		invisiball_state = 0;
		return;
	}

	#define RATIO 14.0

	const bool draw_visi_ball = check_invisi_ball && (
		fabs((b->y - LIMIT) / b->vy) < RATIO ||
		fabs((b->y - (MAX_Y - LIMIT)) / b->vy) < RATIO ||
		fabs(b->x - DEF_W / 2.0) < RATIO ||
		b->x < LIMIT * 2 ||
		b->x > DEF_W - LIMIT * 2
	);

	//cout << "R: " << fabs((b->y - LIMIT) / b->vy) << " || " << fabs((b->y - (MAX_Y - LIMIT)) / b->vy) << endl;

	if (draw_visi_ball) {
		invisiball_state = 1;
		return;
	}

	invisiball_state = 2;
	
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

Bonus::Bonus(PongGame *game, BonusType bonus_type) : 
	Element(game),
	bonus_type(bonus_type),
	cooldown(600 + game->rnd.int_random(0, 1400)) 
{

	this->radius = INITIAL_RADIUS;//provisional

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

	static int prev_inc = 0; //theorically this should be per-instance variable
	auto ball = this->game->ball;

	//double inc = 1 + 1 + sin((double)this->game->tick / 100.0);
	if (ball->get_vx() < 0) {
		return; // this work because ia is always on the right side
	}

	if (ball->get_invisiball_state() == 2) {
		y += prev_inc / 2;
	} else {

		const int inc_module = 1 + this->game->rnd.int_random(0, 1);
		int inc_y = 0;

		if (y > ball->get_y()) inc_y = -inc_module;
		else if (y < ball->get_y()) inc_y = inc_module;
		y += inc_y;
		prev_inc = inc_y;

	}
	
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

void PlayerP::set_com_txt(const string& txt) {

	this->com_txt = txt;
	this->com_txt_y = 150;
	
}

void PlayerP::give_bonus(BonusType bonus_type) {

	if (bonus_type == BONUS_LONG) {
		medlen += 7;
		this->set_com_txt("LONGERRR");
	}

	else if (bonus_type == BONUS_BALL) {
		this->bonus_timers[BONUS_BALL] = 800;
		this->set_com_txt("SPECIAL BALL");
	}

	else if (bonus_type == BONUS_INVI) {
		this->bonus_timers[BONUS_INVI] = 800;
		this->set_com_txt("INVISI BALL");
	}

}


/*
// OLD rand implementation:
uint32_t count_r = 0;
double PongGame::random(double min, double max, bool rand_sign, bool include_max) {

	double real_max = (double)mt19937::max() + (include_max ? 0 : 1);

	double rnd_factor = (double)(*this->mt)() / real_max;
	
	double num = min + rnd_factor * (max - min);
	
	if (rand_sign && (*this->mt)() > mt19937::max() / 2) {
		num = -num;
	}
	
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
	
	return res;

}
*/
