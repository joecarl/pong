#include "utils.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include <string>
#include <stdio.h>

using namespace std;

float random_ex(int max, float min){

	float num = 0.0;
	while(num < 0.1 + min * 10 && num > -(0.1 + min * 10))
		num = rand() % (20 * max) - 10 * max;
	return num / 10;

}

#define FREC_MUESTREO 48000

int last_pos = 0;
int16_t audio[FREC_MUESTREO * 5];//un audio de 5 seg máx...

void PlaySound(int nota, float time, int octava) {

	float frec= 440.0 * pow(2.0, (float) (octava - 3.0 + (nota - 10.0) / 12.0));
	int ciclos = frec * time;//obtenemos un numero de ciclos y lo guardamos como valor entero
	time = float(ciclos) / frec;
	unsigned int samples = FREC_MUESTREO * time / 1000.0;
	
	for(unsigned int i = 0; i < samples; i++){

		audio[last_pos + i] = 30000 * sin(frec * 2 * 3.14 * i / FREC_MUESTREO) * sin(3.14 * i / samples);
	
	}
	
	last_pos += samples;
	
}

string GetWaitString(){

	static string pts;
	static int resize_timer, pts_len = 0;
	
	resize_timer = ++resize_timer % 30;
	
	if(resize_timer == 19){
		pts = "...";
		pts.resize(++pts_len % 4);
	}
	return pts;
}

void PlayAudio(float volumen, ALLEGRO_PLAYMODE mode){

	al_stop_samples();
	ALLEGRO_SAMPLE *beep = NULL;

	uint16_t* buff = (uint16_t*)al_malloc(last_pos*sizeof(uint16_t));
	if (volumen > 1) volumen = 1;
	else if (volumen < 0) volumen = 0;
	for (int i = 0; i < last_pos; i++) {
		buff[i] = audio[i] * volumen;//*sin(3.140*i/last_pos);
	}

	beep = al_create_sample (buff, last_pos, FREC_MUESTREO, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_1, true);
   
	if(beep != NULL){
		//printf("construyendo sample...\n");
		
		al_play_sample(beep, 1.0, 0, 1.0, mode, NULL);
		//al_rest(last_pos/FREC_MUESTREO);
		//al_stop_sample(&sampleid);
		//al_destroy_sample(beep);
	}

	last_pos = 0;

}

#define o 3
void PlayExorcista(){

	PlaySound(Mi, 100, o);
	PlaySound(La, 200, o);
	PlaySound(Mi, 100, o);
	PlaySound(Si, 200, o);
	PlaySound(Mi, 100, o);
	PlaySound(Sol, 200, o);
	PlaySound(La, 200, o);
	PlaySound(Mi, 100, o);
	PlaySound(Do, 200, o + 1);
	PlaySound(Mi, 100, o);
	PlaySound(Re, 200, o + 1);
	PlaySound(Mi, 100, o);
	PlaySound(Si, 200, o);
	PlaySound(Do, 200, o + 1);
	PlaySound(Mi, 100, o);
	PlaySound(Si, 200, o);
	PlayAudio(1, ALLEGRO_PLAYMODE_LOOP);

}

/*
void ShowKeyBoardMatrix(){
	int i,j;
	acquire_screen();
	clear_keybuf();
	for(i=0;i<3;i++){
		for(j=0;j<10;j++){
			char a[2];
			a[1]='\0';
			if(key[KEY_A+i*10+j])a[0]='1';
			else a[0]='0';

			textout_ex(screen,font,a,(j+1)*10,(i+1)*10,makecol(255,255,255),makecol(0,0,0));
		}
	}
	release_screen();
}
*/

JC_TEXTINPUT::JC_TEXTINPUT(ALLEGRO_FONT* fuente){
	
	font = fuente;//al_load_ttf_font("font.ttf", 2*9, 0);
	Reset();

}

void JC_TEXTINPUT::Reset(){

	edittext = "";
	iter = edittext.begin();
	caret = 0;
	caret_time = 0;
	insert = true;
	active = false;
	destino = NULL;

}

void  JC_TEXTINPUT::ProcessKey(wchar_t ASCII, int control_key) {
	if(ASCII >= 32 && ASCII <= 126) {
		//printf("ascii code pressed\n");
		// add the new char, inserting or replacing as need be
		if(insert || iter == edittext.end())
			iter = edittext.insert(iter, ASCII);
		else
			edittext.replace(caret, 1, 1, ASCII);
		
		// increment both the caret and the iterator
		caret++;
		iter++;
	}
	// some other, "special" key was pressed; handle it here
	else switch(control_key){

		case ALLEGRO_KEY_DELETE://delete
			if(iter != edittext.end()) iter = edittext.erase(iter);
			break;
			
		case ALLEGRO_KEY_BACKSPACE://backspace
			if(iter != edittext.begin())
			{
				caret--;
				iter--;
				iter = edittext.erase(iter);
			}
			break;
			
		case ALLEGRO_KEY_RIGHT:
			if(iter != edittext.end())   caret++, iter++;
			break;
			
		case ALLEGRO_KEY_LEFT:
			if(iter != edittext.begin()) caret--, iter--;
			break;
			
		case ALLEGRO_KEY_INSERT:
			insert = !insert;
			break;
			
		default:
			break;
	}

}

void  JC_TEXTINPUT::Draw(int x, int y){

	al_draw_text(font, WHITE, x, y, ALLEGRO_ALIGN_LEFT, edittext.c_str());

	if (caret_time < 30) {

		char text_caret[100]; 
		strcpy(text_caret, edittext.c_str());
		text_caret[caret] = '\0';
		int length = al_get_text_width(font, text_caret);
		al_draw_line(
			x + length + 2,
			y,
			x + length + 2,
			y + 2 * 6,
			WHITE, 
			1
		);
	}

	caret_time++;

	if (caret_time > 60) {
		caret_time = 0;
	}

}

void JC_TEXTINPUT::Start(char* dest){
	Reset();
	destino = dest;
	active = true;
}

void  JC_TEXTINPUT::Finish(){

	if (destino != NULL) {
		strcpy(destino, edittext.c_str());
		destino = NULL;
		active = false;
	} else {
		throw 1;
	}
}

const char* JC_TEXTINPUT::GetValue(){
	return edittext.c_str();
}


