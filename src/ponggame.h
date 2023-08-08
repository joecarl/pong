#ifndef PONGGAME_H
#define PONGGAME_H

#include <random>
#include <queue>
#include <ctime>

#define GROSOR  3
#define MEDLEN  9
#define RADIUS  6
#define LIMIT  15
#define MAX_Y 200
#define DEF_H 200
#define DEF_W 320


enum BonusType {
	BONUS_NONE = 0,
	BONUS_LONG,
	BONUS_IMPA,
	BONUS_BALL,
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

	std::mt19937 *mt;

	uint_fast32_t seed;

	unsigned int control_mode = CONTROLMODE_NONE;

	bool paused = false, finished = false;

	PlayerP *players[2];

	Ball *ball;

	Bonus *bonus[2];

	unsigned int tick = 0;

	std::queue<std::string> messages;
	
	PongGame(uint_fast32_t _seed = std::time(nullptr));

	void restart();

	void toggle_pause();

	void iniciar_punto(int first);

	void give_score(PlayerP* pl, int score);

	void process_tick();

	void add_message(const std::string& evt_msg);
	
	/**
	 * Generates a decimal pseudo random number between min and max
	 * @param min minum retornable value
	 * @param max maximum retornable value
	 * @param rand_sign if true, the returned value sign will be positive or negative at 50/50 chance
	 * @param include_max if true, the returned value will be in the range [min, max], otherwise it will be in the range [min, max)
	 */
	double random(double min, double max, bool rand_sign = false, bool include_max = true);

	/**
	 * Generates an integer pseudo random number in the range [min, max]
	 * @param min minum retornable value
	 * @param max maximum retornable value
	 * @param rand_sign if true, the returned value sign will be positive or negative at 50/50 chance
	 */
	int int_random(int min, int max, bool rand_sign = false);


};


class Element {

public:

	bool stat = false;

	double x, y, x00, y00, radius;
	
	double vx, vy, t;

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

	void set_pos(int px, int py) {x = x00 = px; y = y00 = py;}

	double get_vx() { return vx; }
	
	virtual void on_player_hit(PlayerP *pl) {}
};


class Ball: public Element {
	
public:

	Ball(PongGame* game);

	void on_player_hit(PlayerP *pl);
	
	void preprocess();

};


class Bonus: public Element {
	
	public:

	int bonus_type;

	int cooldown = 500;

	Bonus(PongGame* game, int bonus_type);
	
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

	void give_bonus(int bonus_type);
 
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
