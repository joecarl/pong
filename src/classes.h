#ifndef CLASSESH
#define CLASSESH

#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

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
#define ALPHA_COLOR al_map_rgb(255,0,255)

#ifdef __APPLE__
#define LONG_DIR "../Resources/long.bmp"
#define BALL_DIR "../Resources/ball.bmp"
#define LOGO_DIR "../Resources/pong.bmp"
#define FONT_DIR "../Resources/font.ttf"

#elif defined _WIN32 || defined _WIN64
#define LONG_DIR "resources/long.bmp"
#define BALL_DIR "resources/ball.bmp"
#define LOGO_DIR "resources/pong.bmp"
#define FONT_DIR "resources/font.ttf"
#endif


class playerP;
class element;

class PongGame{

public:

    int bonus_time[2] = {-1, -1};

    float scale = 1.0;

    unsigned int numPlayers;

    bool paused = false, finished = false;

    playerP* players[2];

	element *ball, *bonus[2];

    
    PongGame(float sc, ALLEGRO_FONT* font);

    void restart();

    void togglePause();

    void iniciarPunto(int first);

    void giveScore(playerP* pl, int score);

    void processTick(bool* keys);

};

class element{
	
	ALLEGRO_BITMAP* sprite;

    float x, y, x00, y00, radius;
    int stat, bonus_type;
    float vX, vY, t;
    bool isCircle;

    public:

    element(bool iscircle, int bonus, const char *filename);
    void SetParameters(float px, float py, float vx, float vy,int st=1); //Configures to start a new movement
    void ProcessColliding(int scale, int plyrNum, playerP* players[]);
    void Process(int scale, int plyrNum, playerP* players[]);//Moves, tests collinding and draws the element
    int GetStat(){ return stat; }
    void SetStat(int st){ stat = st; }
    int GetX(){ return x; }
    int GetY(){ return y; }
    void SetPos(int px, int py){x = x00 = px; y = y00 = py;}
    float GetvX(){ return vX; }
    void Draw(int scale);
};


class playerP{

    ALLEGRO_FONT* font;
    int x, y, comTxtY;
    char comTxt[30];

    public:

    int score, medlen, bonus_ball, racha;

    playerP(int px, int py, ALLEGRO_FONT* _font);
    void GiveBonus(int bonus_type);
    void Draw(int scale);
    void LockLimit(int scale);
    void ControlMove(int scale, int key_up, int key_down, int key_up2 = 0, int key_down2 = 0);
    void MoveIA(int plyrNum, element *ball, int scale);
    void SetX(int px){ x = px; }
    void SetY(int py){ y = py; }
    int GetX(){ return x; }
    int GetY(){ return y; }

};

#endif
