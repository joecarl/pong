#ifndef PONGGAME_H
#define PONGGAME_H

//#include <random>
#include "randomgenerator.h"
#include <queue>
#include <ctime>
#include <string>

#define MAX_SCORE 11
#define INITIAL_SPEED 2
#define INITIAL_RADIUS  6
#define GROSOR  3
#define MEDLEN  9
#define LIMIT  15
#define MAX_Y 200
#define DEF_H 200
#define DEF_W 320


enum BonusType {
	BONUS_NONE = -1,
	BONUS_LONG,
	BONUS_BALL,
	BONUS_INVI,
	BONUS_MAX
};

enum Control {
	CONTROL_NONE = 0,
	CONTROL_MOVE_UP,
	CONTROL_MOVE_DOWN,
	CONTROL_MAX
};

class PlayerP;
class Element;
class Ball;
class Bonus;


enum {
	CONTROLMODE_NONE = 0,
	CONTROLMODE_SINGLE_PLAYER,
	CONTROLMODE_TWO_PLAYERS,
	CONTROLMODE_TRAINING,
	CONTROLMODE_DEBUG
};


class PongGame {

public:

	RandomGenerator rnd;

	unsigned int control_mode = CONTROLMODE_NONE;

	bool paused = false, finished = false;

	uint16_t warmup = 0;

	PlayerP *players[2];

	Ball *ball;

	Bonus *bonus[BONUS_MAX];

	unsigned int tick = 0;

	std::queue<std::string> messages;
	
	PongGame(uint_fast32_t _seed = std::time(nullptr));

	void restart();

	void toggle_pause();

	void iniciar_punto(int first);

	void give_score(PlayerP* pl, int score);

	void process_tick();

	void add_message(const std::string& evt_msg);

	uint8_t get_winner();
	
};


class Element {

public:

	bool stat = false;

	double x, y, radius;
	
	double vx, vy;

	PongGame *game;

	Element(PongGame *game);

	void set_parameters(double px, double py, double vx, double vy, int st = 1); //Configures to start a new movement
	
	void process_colliding();

	virtual void preprocess() {}

	void process();//Moves, tests collinding and draws the element

	int get_stat() { return stat; }

	void set_stat(int st) { stat = st; }

	int get_x() { return x; }

	int get_y() { return y; }

	void set_pos(int px, int py) { x = px; y = py; }

	double get_vx() { return vx; }
	
	virtual void on_player_hit(PlayerP *pl) {}
};


class Ball: public Element {

	uint8_t invisiball_state = 0;

	void calc_invisiball_state();
	
public:

	Ball(PongGame* game);

	void on_player_hit(PlayerP *pl);
	
	void preprocess();

	uint8_t get_invisiball_state() { return this->invisiball_state; }

};


class Bonus: public Element {
	
	public:

	BonusType bonus_type;

	int cooldown;

	Bonus(PongGame* game, BonusType bonus_type);
	
	void on_player_hit(PlayerP *pl);
	
};


class PlayerP {

	PongGame *game;	

public:
	
	int x, y, com_txt_y;
	
	std::string com_txt;

	int score, medlen, racha;

	unsigned int bonus_timers[BONUS_MAX];

	bool controls[CONTROL_MAX];

	PlayerP(PongGame *pong_game, int px, int py);

	void set_com_txt(const std::string& txt);

	void give_bonus(BonusType bonus_type);
 
	void lock_limit();
	
	void reset();

	void control_move();

	void move_ia();

	void move_perfect();

	void set_x(int px) { x = px; }

	void set_y(int py) { y = py; }

	int get_x() { return x; }

	int get_y() { return y; }

};

#endif
