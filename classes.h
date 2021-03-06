#ifndef CLASSESH
#define CLASSESH

#include <stdio.h>
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
#define FONT_DIR "../Resources/arial.ttf"

#elif defined _WIN32 || defined _WIN64
#define LONG_DIR "long.bmp"
#define BALL_DIR "ball.bmp"
#define LOGO_DIR "pong.bmp"
#define FONT_DIR "arial.ttf"
#endif


class playerP;
class element;

class element{
  ALLEGRO_BITMAP* sprite;

  // palette;

    float x, y, x00, y00, radius;
    int stat, bonus_type;
    float vX,vY,t;
    bool isCircle;

    public:

    element(bool iscircle,int bonus,const char *filename){
        vX=vY=t=0;
        x=y=-100;
        if(!iscircle){
            printf("cargando bitmap...\n");
            sprite=al_load_bitmap(filename);
            al_convert_mask_to_alpha(sprite, ALPHA_COLOR);
            if(!sprite)printf("error al cargar bitmap\n");
        }
        isCircle=iscircle;
        bonus_type=bonus;
        radius=RADIUS;
    }
    void SetParameters(float px, float py, float vx, float vy,int st=1); //Configures to start a new movement
    void ProcessColliding(int scale,int plyrNum,playerP* players[]);
    void Process(int scale,int plyrNum,playerP* players[]);//Moves, tests collinding and draws the element
    int GetStat(){return stat;}
    void SetStat(int st){stat=st;}
    int GetX(){return x;}
    int GetY(){return y;}
    void SetPos(int px, int py){x=x00=px;y=y00=py;}
    float GetvX(){return vX;}
    void Draw(int scale);
};


class playerP{
    ALLEGRO_FONT* font;
    int x, y, comTxtY;
    char comTxt[30];

    public:
    int score, medlen, bonus_ball;
    playerP( int px, int py,ALLEGRO_FONT* _font){
        font=_font;
        x=px;
        y=py;
        medlen=MEDLEN;
        //bonus=0;
        score=0;
        comTxtY=-40;//DESACTIVADA
        bonus_ball=0;
    }
    void GiveBonus(int bonus_type);
    void Draw(int scale);
    void LockLimit(int scale);
    void ControlMove(int scale, int key_up, int key_down,int key_up2=0, int key_down2=0);
    void MoveIA(int plyrNum, element *ball,int scale);
    void SetX(int px){x=px;}
    void SetY(int py){y=py;}
    int GetX(){return x;}
    int GetY(){return y;}
};

#endif
