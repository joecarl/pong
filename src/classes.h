#ifndef CLASSESH
#define CLASSESH

#include <random>

#define GROSOR  3
#define MEDLEN  9
#define RADIUS  6
#define LIMIT  15
#define MAX_Y 200
#define DEF_H 200
#define DEF_W 320
#define BONUS_LONG 1
#define BONUS_IMPA 2
#define BONUS_BALL 3

class PlayerP;
class Element;
class Ball;
class Bonus;

class PongGame{

public:

	std::mt19937 *mt;

	int bonus_time[2] = {-1, -1};

	unsigned int numPlayers;

	bool paused = false, finished = false;

	PlayerP *players[2];

	Ball *ball;

	Bonus *bonus[2];

	
	PongGame();

	void restart();

	void togglePause();

	void iniciarPunto(int first);

	void giveScore(PlayerP* pl, int score);

	void processTick(bool* keys);

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
	int intRandom(int min, int max, bool rand_sign = false);


};


class Element{

public:

	int stat;

	float x, y, x00, y00, radius;
	
	float vX, vY, t;

	PongGame *game;

	Element(PongGame *game);

	void setParameters(float px, float py, float vx, float vy, int st = 1); //Configures to start a new movement
	
	void processColliding();

	virtual void preprocess(){}

	void process();//Moves, tests collinding and draws the element

	int getStat(){ return stat; }

	void setStat(int st){ stat = st; }

	int getX(){ return x; }

	int getY(){ return y; }

	void setPos(int px, int py){x = x00 = px; y = y00 = py;}

	float getvX(){ return vX; }
	
	virtual void onPlayerHit(PlayerP *pl){}
};


class Ball: public Element{
	
public:

	Ball(PongGame* game);

	void onPlayerHit(PlayerP *pl);
	
	void preprocess();

};


class Bonus: public Element{
	
	public:

	int bonus_type;

	Bonus(PongGame* game, int bonus_type);
	
	void onPlayerHit(PlayerP *pl);
	
};


class PlayerP{

	PongGame *game;	

public:
	
	int x, y, comTxtY;
	char comTxt[30];
	int score, medlen, racha;

	float bonus_ball;

	PlayerP(PongGame *pongGame, int px, int py);

	void giveBonus(int bonus_type);
 
	void lockLimit();

	void controlMove(int key_up, int key_down, int key_up2 = 0, int key_down2 = 0);

	void moveIA();

	void setX(int px){ x = px; }

	void setY(int py){ y = py; }

	int getX(){ return x; }

	int getY(){ return y; }

};

#endif
