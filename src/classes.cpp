#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include "classes.h"
#include "funciones.h"
#include <math.h>

element::element(bool iscircle, int bonus, const char *filename){

	vX = vY = t = 0;
	x = y = -100;

	if(!iscircle){
		printf("cargando bitmap...\n");
		sprite = al_load_bitmap(filename);
		al_convert_mask_to_alpha(sprite, ALPHA_COLOR);
		if(!sprite) printf("error al cargar bitmap\n");
	}

	isCircle = iscircle;
	bonus_type = bonus;
	radius = RADIUS;

}

void element::Process(int scale,int plyrNum,playerP* players[]){

	if(stat){
		x = x00 + vX * t;
		y= y00 + vY * t;
		t++;

		if(isCircle){
			if(players[0]->bonus_ball && !players[1]->bonus_ball)
				radius = (scale * 320 - x) / 20 + 2;
			else if(players[1]->bonus_ball && !players[0]->bonus_ball)
				radius = x / 20 + 2;
			else
				radius= RADIUS;
		}
		ProcessColliding(scale,plyrNum,players);
		//al_set_blender(ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA, al_map_rgb(255,255,255));
		if(isCircle)
			al_draw_filled_circle( x, y, radius*scale, al_map_rgb( 255,255, 255));
		else
			al_draw_bitmap( sprite,x-10,y-10,0);// x-sprite->w/2, y-sprite->h/2);
		if(x < -scale*80 || x > scale * DEF_W + 50)
			stat=0;

	//ProcessColliding(scale,plyrNum,players);
	}
}

void element::Draw(int scale){
	if(isCircle)
		al_draw_filled_circle(x, y, radius*scale, al_map_rgb( 255,255, 255));
	else
		al_draw_bitmap(sprite, x - 10, y - 10, 0);// x-sprite->w/2, y-sprite->h/2);
}

void element::SetParameters(float px, float py, float vx, float vy,int st){

	x = x00 = px;
	y = y00 = py;
	vX = vx;
	vY = vy;
	t = 0;
	stat = st;

}

void element::ProcessColliding(int scale,int plyrNum,playerP* players[]){

	//CHOQUE CON LA PARTE DE ARRIBA
	if(y <= scale*(radius+LIMIT))
		SetParameters(this->x, scale*(radius+LIMIT+1), this->vX, -this->vY, stat);

	//CHOQUE CON LA PARTE DE ABAJO
	if(y >= scale*(MAX_Y-radius-LIMIT))
		SetParameters(this->x, scale*(MAX_Y-radius-LIMIT-1), this->vX, -this->vY, stat);

	//CHOQUE CON LA PALA IZDA
	if(x <= scale * (radius + GROSOR)){
		if(fabs(y-players[0]->GetY()) < scale * (players[0]->medlen+radius) && x > scale * (GROSOR)){
			if(isCircle){
				if(y-players[0]->GetY()>scale*2) vY += 1;
				else if (y-players[0]->GetY()>scale*1) vY += 0.5;
				if(y-players[0]->GetY()<-scale*2) vY -= 1;
				if (y-players[0]->GetY()<-scale*1) vY -= 0.5;
					PlaySound(Mi,40,4);
					PlayAudio();
				SetParameters(scale*(radius+GROSOR+1),y,-vX,vY,stat);
			} else {
				stat = 0;
				players[0]->GiveBonus(bonus_type);
			}
		}
	}

	//CHOQUE CON LA PALA DCHA
	int grosorB;
	if(plyrNum!=0) grosorB = GROSOR;
	else grosorB = 0;

	if(x >= scale*(320-radius-grosorB)){
		if(fabs(y-players[1]->GetY()) < scale * (players[1]->medlen+radius) && x < scale * (320-grosorB)){
			if(isCircle){
				if(plyrNum!=0){
					if(y-players[1]->GetY()>scale*2)vY+=1;
					else if (y-players[1]->GetY()>scale*1)vY+=0.5;
					if(y-players[1]->GetY()<-scale*2)vY-=1;
					if (y-players[1]->GetY()<-scale*1)vY-=0.5;
					PlaySound(Mi,40,4);
					PlayAudio();
				}
				SetParameters(scale*(320-radius-grosorB-1),y,-vX,vY,stat);
			} else {
				stat=0;
				players[1]->GiveBonus(bonus_type);
			}
		}
	}
}

//------------------------------------------------------------------------------

playerP::playerP(int px, int py, ALLEGRO_FONT* _font){
	font = _font;
	x = px;
	y = py;
	medlen = MEDLEN;
	//bonus = 0;
	score = 0;
	comTxtY = -40;//DESACTIVADA
	bonus_ball = 0;
}

void playerP::LockLimit(int scale){

	if(y >= scale * (MAX_Y - medlen - LIMIT))
		y = scale * (MAX_Y - medlen - LIMIT - 1);
	if(y <= scale * (medlen + LIMIT))
		y = scale * (medlen + LIMIT + 1);

}

void playerP::Draw(int scale){

	int medln = this->medlen;
	
	al_draw_filled_rectangle(
		x - 2, 
		y - scale * medln, 
		x + 2, 
		y + scale * medln, 
		WHITE
	);
	al_draw_filled_rectangle(
		x - 1,
		y - scale * medln - 1, 
		x + 1, 
		y + scale * medln+1, 
		WHITE
	);
	 
	static int bonus_ball_decrease = 10;

	if(bonus_ball){
		if(x < 100)	al_draw_filled_rectangle(0, 16, 158.0 * bonus_ball / 80.0, 20, al_map_rgb(0, 200, 50));
		else		al_draw_filled_rectangle(DEF_W, 16, DEF_W - 158.0 * bonus_ball / 80.0, 20, al_map_rgb(0, 200, 50));
		
		bonus_ball_decrease--;
		if (bonus_ball_decrease <= 0){
			bonus_ball_decrease = 10;
			bonus_ball--;
		}
	}

	if(comTxtY >- 40){
		int txtX;
		if(x<100) txtX = scale * 60;
		else txtX = scale * 220;
		al_draw_text(font, al_map_rgb( 150-comTxtY,150-comTxtY*4, 255), txtX, comTxtY, ALLEGRO_ALIGN_CENTER, comTxt);//al_map_rgb(255,0,255) );
		comTxtY -= 2;
	}

}

void playerP::MoveIA(int plyrNum, element *ball,int scale){

	if(plyrNum == 1){
		if(y > ball->GetY()) y -= scale * (1 + rand() % 2);
		if(y < ball->GetY()) y += scale * (1 + rand() % 2);
	}
	else if(plyrNum == 0)
		y = ball->GetY();

	LockLimit(scale);

}

void playerP::ControlMove(int scale, int key_up, int key_down,int key_up2, int key_down2){

	if(key_up	|| key_up2) y -= scale*2;//ch='w';
	if(key_down || key_down2) y += scale*2;//ch='s';
	LockLimit(scale);

}

void playerP::GiveBonus(int bonus_type){

	if(bonus_type == BONUS_LONG){
		medlen += 7;
		strcpy(comTxt,"LONGERRR");
		comTxtY = 150;
	}

	if(bonus_type == BONUS_IMPA){
		strcpy(comTxt,"UNSTOPABLE");
		comTxtY = 150;
	}

	if(bonus_type == BONUS_BALL){
		strcpy(comTxt,"SPECIAL BALL");
		comTxtY = 150;
		bonus_ball = 80;
	}

}
