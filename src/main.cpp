#include <stdio.h>
#include <iostream>
#include <allegro5/allegro.h>

#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <string>
#include <math.h>
#include "classes.h"
#include "utils.h"
#include "ioclient.h"
#define ESC	27

#define INIX 160
#define INIY 100
#define FPS	 60.0

enum{
	MENU = 0,
	GAME = 1,
	OVER = 2,
	CONN = 3,
	MAX_SCREENS
};
bool keys[300];

ALLEGRO_FONT* font;

ALLEGRO_KEYBOARD_STATE keyState;

void MostrarMenu(int sc, ALLEGRO_BITMAP* logo);
void IniciarPunto(int plyrNum, int first, element *ball, playerP *players[], int sc);
void ShowScores(int plyrNum, int scale, playerP *players[]);
void DCourt(int scale);
void AskRetry(playerP* players[], int scale);

void keyb_task(){
	while (1){

		al_get_keyboard_state(&keyState);
		for (int i = 0; i < ALLEGRO_KEY_MAX; i++) {
			keys[i] = al_key_down(&keyState, i);
		}
		//usleep(1000*20);
		boost::this_thread::sleep(boost::posix_time::milliseconds(20));
	}
}

void timer_task(int *ms){
	while (1) {
		//usleep(1000);
		boost::this_thread::sleep(boost::posix_time::milliseconds(1));
		(*ms)++;
	}
}

void MostrarMenu(int sc, ALLEGRO_BITMAP* logo){

	al_draw_bitmap(logo, (sc-1)*DEF_W/2, (sc-1)*50, 0);
	al_draw_text( font, al_map_rgb( 255, 255, 255), sc*DEF_W/2, sc*110,ALLEGRO_ALIGN_CENTER, "Recreated by: Jose Carlos HR" );
	al_draw_text( font, al_map_rgb( 255, 255, 255), sc*DEF_W/2, sc*140,ALLEGRO_ALIGN_CENTER, "1:One Player  2:Two Players" );
	al_draw_text( font, al_map_rgb( 255, 255, 255), sc*DEF_W/2, sc*150,ALLEGRO_ALIGN_CENTER, "3:Training    4:Play online" );
	al_draw_text( font, al_map_rgb( 255, 255, 255), sc*DEF_W/2, sc*165,ALLEGRO_ALIGN_CENTER, "ESC: Quit" );

}

void IniciarPunto(int plyrNum, int first, element *ball, playerP *players[], int sc){

	int tvx = 0;
	if(first == 1){
		players[0]->score = 0; 
		players[1]->score = 0;
		while(tvx == 0)
			tvx = sc * 2 * (rand() % 3 - 1);
	}
	else tvx = (int)ball->GetvX();
	players[0]->medlen = MEDLEN; 
	players[1]->medlen = MEDLEN;
	players[0]->bonus_ball = 0; players[1]->bonus_ball = 0;
	ball->SetParameters(sc*INIX, sc*INIY, tvx, sc*0.5*(rand()%3-1));
	players[0]->SetY(sc * 40);
	players[1]->SetY(sc * 160);
	printf("iniciado nuevo punto\n");
	
}

void ShowScores(int plyrNum, int scale, playerP *players[]){

	//al_draw_text(font,al_map_rgb( 255, 0, 0),scale*320/2, scale*2, ALLEGRO_ALIGN_CENTER, "Press ESC to Main Menu" );
	if(plyrNum != 0){
		al_draw_textf( font, al_map_rgb( 255, 0, 0), scale*25, scale*186, ALLEGRO_ALIGN_LEFT, "SCORE:%d",players[0]->score );
		al_draw_textf( font, al_map_rgb( 255, 0, 0), scale*240, scale*186, ALLEGRO_ALIGN_LEFT, "SCORE:%d",players[1]->score );
	} else {
		al_draw_textf( font, al_map_rgb( 255, 0, 0), scale*25, scale*186, ALLEGRO_ALIGN_LEFT, "FAILS:%d",players[1]->score );
	}

}

//DIBUJAR EL CAMPO
void DCourt(int scale){

	al_draw_line(0, scale*LIMIT, scale*DEF_W, scale*LIMIT, al_map_rgb( 255, 255, 255),2);
	al_draw_line(0, scale*(MAX_Y-LIMIT),scale*DEF_W, scale*(MAX_Y-LIMIT), al_map_rgb( 255, 255, 255),2);
	al_draw_line(scale*(320/2-1), scale*LIMIT, scale*(320/2-1), scale*(MAX_Y-LIMIT), al_map_rgb( 255, 255, 255),2);
	al_draw_line(scale*(320/2+1), scale*LIMIT, scale*(320/2+1), scale*(MAX_Y-LIMIT), al_map_rgb( 255, 255, 255),2);

}

void AskRetry(playerP* players[], int scale){

	int winner;
	if(players[0]->score > players[1]->score) winner = 1;
	else winner = 2;
	al_draw_textf(font, al_map_rgb( 255, 0, 0), scale*320/2, scale*54, ALLEGRO_ALIGN_CENTER, "WINNER: PLAYER %d", winner);
	al_draw_text (font, al_map_rgb( 255, 0, 0), scale*320/2, scale*70, ALLEGRO_ALIGN_CENTER, "REPLAY? (Y/N)" );
}

string GetData(string pkg,string field){

	size_t pos = pkg.find(field);
	if(pos == std::string::npos)
		return "";
	pkg = &pkg[pos + field.length() + 1];
	pos = pkg.find(" ");
	if(pos != std::string::npos)
		pkg.resize(pos);
	return pkg;
}

//------------------------------------------------------------------------------
//--------------------------- PROGRAMA PRINCIPAL -------------------------------
//------------------------------------------------------------------------------

int main(int argc, char **argv){
	
	bool paused = 0, start_connection = 0;
	for (unsigned int i = 0; i < sizeof(keys); i++) keys[i] = 0;
	int gScreen, players;
	float scale = 1.0;
	int bonus_time[2] = {-1, -1};
	int racha[2] = {0, 0};
	int load_screen[MAX_SCREENS];
	playerP *player[2];
	element *Ball, *bonus[2];
	ioClient connection;
	load_screen[MENU] = 1;
	//-------------------NET----
	bool	isClient = false, sendping;
	char	addr[20];
	int		pingCounter=1;
	int		pingms;
	
	addr[0] = '\0';
	
	al_init_image_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_primitives_addon();
	
	bool finish = false; //, draw= true;
	float fps = 0;
	float frames_done = 0;
	double old_time = 0; //al_get_time();
	int resX = DEF_W, resY = DEF_H;
	
	if(!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
	}
	printf("allegro initialized\n");

	al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW );
	al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_REQUIRE);

	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_DISPLAY_MODE disp_data;
	ALLEGRO_BITMAP* buffer;
	
	al_get_display_mode(al_get_num_display_modes()-1, &disp_data);
	//1024x768
	//1920,windowHeight=1080
	int windowWidth = disp_data.width, windowHeight = disp_data.height, screenWidth = scale * resX, screenHeight = scale * resY;
	display = al_create_display(windowWidth, windowHeight);
	if(!display){
		fprintf(stderr, "failed to create display!\n");
		return -1;
	}
	
	buffer = al_create_bitmap(screenWidth, screenHeight);
	// calculate scaling factor
	float sx = float(windowWidth) / screenWidth;
	float sy = float(windowHeight) / screenHeight;
	float scaled = std::min(sx, sy);
	// calculate how much the buffer should be scaled
	int scaleW = screenWidth * scaled;
	int scaleH = screenHeight * scaled;
	int scaleX = (windowWidth - scaleW) / 2;
	int scaleY = (windowHeight - scaleH) / 2;
	
	printf("display created\n");
	/*
	for(int i=0; i< al_get_num_display_modes(); i++){
		al_get_display_mode(i, &disp_data);
		printf("Resolution: %dx%d\n",disp_data.width,disp_data.height);
		
	}
	*/

	if (!al_install_audio()) {
		fprintf(stderr, "Could not init sound!\n");
		return -1; 
	}
	if (!al_reserve_samples(1)) {
		fprintf(stderr, "Could not reserve samples!\n");
		return -1; 
	}
	
	font = al_load_ttf_font(FONT_DIR, scale*10, ALLEGRO_TTF_MONOCHROME);
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
	Ball = new element (true, 0, "");
	player[0] = new playerP(scale * 1, 50, font);
	player[1] = new playerP(scale * (DEF_W-GROSOR/2), 50, font);
	bonus[0] = new element(false, BONUS_LONG, LONG_DIR);
	bonus[1] = new element(false, BONUS_BALL, BALL_DIR);
	JC_TEXTINPUT input(font);
	
	gScreen = MENU;
	printf("elements loaded\n");

	al_install_keyboard();
	
	ALLEGRO_BITMAP* logo = al_load_bitmap(LOGO_DIR);
	ALLEGRO_TIMER* timer = al_create_timer(1.0 / FPS);
	ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	al_start_timer(timer);
	al_hide_mouse_cursor(display);
	boost::thread kbth(keyb_task);
	printf("settings done\n");
	ALLEGRO_EVENT events;
	string server;
	int milliseconds = 0;
	boost::thread th_tim(timer_task, &milliseconds);
	int delayer;
	//---------------BUCLE PRINCIPAL---------------
	do{
		al_wait_for_event(event_queue, &events);
		if(events.type == ALLEGRO_EVENT_KEY_CHAR)
		{
			if(input.active){
				
				if (events.keyboard.keycode != ALLEGRO_KEY_ENTER) {
					input.ProcessKey(events.keyboard.unichar, events.keyboard.keycode);
				} else {
					input.Finish();
					start_connection = true;
				}
			}
		}
		if(events.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
			break;
		
		//---------------PANTALLAS---------------
		if(events.type == ALLEGRO_EVENT_TIMER)
		{
			al_set_target_bitmap(buffer);
			al_clear_to_color(al_map_rgb(0, 0, 0));
			double game_time = al_get_time();
			if(game_time - old_time >= 1.0)
			{
				fps = frames_done / (game_time - old_time);
				frames_done = 0;
				old_time = game_time;
			}
			frames_done++;
			
			//-------------GAME--------------
			if(gScreen == GAME)
			{
				if(load_screen[GAME]){
					
					PlaySound(Do, 400);
					PlaySound(Re, 200);
					PlaySound(La, 100);
					PlaySound(Si, 100);
					PlayAudio();
					IniciarPunto(players, 1, Ball, player, scale);
					delayer = 75;
					load_screen[GAME] = false;
				}
				
				if (delayer-- > 0) {
					al_draw_textf(font, al_map_rgb(255, 0, 0), scale * resX / 2, scale * resY / 2, ALLEGRO_ALIGN_CENTER, "%d", 1 + delayer / 25);
					goto end_section_game;
				}
				if(keys[ALLEGRO_KEY_ESCAPE]){//ESC (SALIR)
					al_rest(0.2);
					gScreen=MENU;
				}
				//NETWORK
				static int counter, time;
				if(isClient){
					
					if ((counter++)%60 == 1){// && sendping){
						time = milliseconds;
						sendping = false;
						connection.SendPacket("PING:" + boost::to_string(pingCounter++) + " \r\n");
					}

					stringstream buff;
					buff << "KEYUP:"<<keys[ALLEGRO_KEY_UP]<<" KEYDOWN:"<<keys[ALLEGRO_KEY_DOWN]<<" \r\n";
					connection.SendPacket(buff.str());
					
					while (const char* data=connection.FetchPacket()) {
						if (GetData(data, "PING") == boost::to_string(pingCounter - 1)) {
							pingms = milliseconds - time;
							sendping = true;
						}
						if(GetData(data, "PING")==""){
							string pkg=data;
							player[0]->SetY(atoi(GetData(pkg, "P1Y").c_str()));
							player[1]->SetY(atoi(GetData(pkg, "P2Y").c_str()));
							player[0]->score=atoi(GetData(pkg, "S1").c_str());
							player[1]->score=atoi(GetData(pkg, "S2").c_str());
							Ball->SetPos(atoi(GetData(pkg, "BX").c_str()), atoi(GetData(pkg, "BY").c_str()));
							
							if(GetData(pkg, "W") == "1" || GetData(pkg, "W") == "2"){
								gScreen = OVER;
								PlaySound(Re, 150, 3);
								PlaySound(Re, 150, 3);
								PlaySound(Re, 200, 3);
								PlaySound(LaSos, 500, 2);
								PlayAudio();
							}
						}
					}

					DCourt(scale);
					al_draw_textf(font, al_map_rgb(255, 0, 0), scale * resX / 2, scale * 2, ALLEGRO_ALIGN_CENTER, "PING:%d", pingms);
					//[END]NETWORK

				} else {

					if(keys[ALLEGRO_KEY_P]){//P (PAUSA)
						al_rest(0.2);
						paused = !paused;
					}
					if (paused) {
						al_draw_text(font, al_map_rgb(0, 200, 100), resX * scale / 2, resY * scale / 2 - 5, ALLEGRO_ALIGN_CENTER, "PAUSA");
						goto end_section_game;
					}
					al_draw_text(font,al_map_rgb( 255, 0, 0),scale*320/2, scale*2, ALLEGRO_ALIGN_CENTER, "Press ESC to Main Menu" );
					if(keys[ALLEGRO_KEY_G]) player[0]->medlen += 1;//DEBUG

					if(players == 2){
						player[1]->ControlMove(scale, keys[ALLEGRO_KEY_UP], keys[ALLEGRO_KEY_DOWN], keys[ALLEGRO_KEY_I], keys[ALLEGRO_KEY_K]);
						player[0]->ControlMove(scale, keys[ALLEGRO_KEY_W], keys[ALLEGRO_KEY_S]);
					} else {
						player[0]->ControlMove(scale, keys[ALLEGRO_KEY_UP], keys[ALLEGRO_KEY_DOWN]);
						player[1]->MoveIA(players,Ball,scale);
					}
					
					//COMPROBAMOS QUE LA BOLA ESTÁ FUERA
					if(Ball->GetX() > scale * (320 + 15) || Ball->GetX() < scale * (-15) ){

						if (Ball->GetX()<-15){
							player[1]->score++;
							racha[1]++;
							racha[0]=0;
						} else {
							player[0]->score++;
							racha[0]++;
							racha[1]=0;
						}

						if(racha[0]>3)
							player[0]->GiveBonus(BONUS_IMPA);
						if(racha[1]>3)
							player[1]->GiveBonus(BONUS_IMPA);
						
						PlaySound(Re, 130);
						PlaySound(Do, 250);
						PlayAudio();
						IniciarPunto(players, 0, Ball, player, scale);
					}
					//[END] COMPROBAMOS QUE LA BOLA ESTÁ FUERA
					
					if((player[0]->score == 11 || player[1]->score == 11) && players != 0){

						gScreen = OVER;
						PlaySound(Re, 150, 3);
						PlaySound(Re, 150, 3);
						PlaySound(Re, 200, 3);
						PlaySound(LaSos, 500, 2);
						PlayAudio();

					}
					
					DCourt(scale);
					
					Ball->Process(scale, players, player);

					for(int i=0; i < 2; i++){
						if(bonus[i]->GetStat() == 0 && bonus_time[i] > 1000 && !(rand() % 100)){
							bonus[i]->SetParameters(100 + rand() % 140, 70 + rand() % 60, random_ex(2, 1.5), random_ex(2, 1.5));
							bonus_time[i] =- 1;
						}
						bonus[i]->Process(scale, players, player);
						if(bonus[i]->GetStat() == 0 && bonus_time[i] == -1)
							bonus_time[i] = 1;
						if(bonus_time>0)
							bonus_time[i]++;
					}
				}
				Ball->Draw(scale);
				ShowScores(players,scale,player);
				al_draw_textf(font, al_map_rgb( 255, 0, 0), scale * 160, scale * 186, ALLEGRO_ALIGN_CENTER, "FPS:%d", int(fps) );
				
				player[0]->Draw(scale);
				if(players != 0)
					player[1]->Draw(scale);
				
				end_section_game:;
			}
			//-----[END] ---GAME----
			
			//------------CONNECTION--------------
			
			if(gScreen == CONN){
				if(keys[ALLEGRO_KEY_ESCAPE]){//ESC (SALIR)
					al_rest(0.2);
					input.active=false;
					gScreen=MENU;
				}
				if(load_screen[CONN]){
					load_screen[CONN] = false;
					input.Start(addr);
					al_flush_event_queue(event_queue);
					connection.Reset();
					
					//
				}
				string pts=GetWaitString();
				
				al_draw_text(font,WHITE,20, 30, ALLEGRO_ALIGN_LEFT, "ENTER SERVER IP ADDRESS or press ");
				al_draw_text(font,WHITE,20, 40, ALLEGRO_ALIGN_LEFT, "enter to connect to default server:");
				if (!input.active && !connection.connected) {
					if(!connection.stopped){
						al_draw_textf(font, WHITE, 20, 60, ALLEGRO_ALIGN_LEFT, "Trying %s %s", server.c_str(), pts.c_str());
						if (start_connection) {
							server = addr;
							if (server == "")
								server = "copinstar.ddns.net";
							connection.Connect(server, "25000");//addr
							start_connection = false;
							
						}
					} else {
						al_draw_text(font,WHITE,20, 60, ALLEGRO_ALIGN_LEFT,"Connection error.");
					}
					
				} else if (connection.connected && !connection.stopped) {

					al_draw_textf(font,WHITE,20, 60, ALLEGRO_ALIGN_LEFT,"Connected to %s", server.c_str());
					al_draw_textf(font,WHITE,20, 75, ALLEGRO_ALIGN_LEFT,"Wait please %s", pts.c_str());
					
					static int delay=0;
					if((delay++) % 40 == 20){
						connection.SendPacket("PING:-1 \r\n");
						if(const char* pkg = connection.FetchPacket()){
							string pk = pkg;
							if(GetData(pkg, "PING") != ""){
								gScreen = GAME;
								isClient = 1;
								cout << "Conexion OK!" << endl;
								sendping = true;
							}
						}
					}
				}
			}

			//----------------OVER----------------
			if(gScreen==OVER){
				AskRetry(player,scale);
				if(keys[ALLEGRO_KEY_Y]){
					gScreen = GAME;
					load_screen[GAME] = true;
				}
				if(keys[ALLEGRO_KEY_N])
					gScreen = MENU;
			}
			
			//----------------MENU----------------
			if(gScreen == MENU){
				static int easteregg = 0;
				if(easteregg++ == 1000) PlayExorcista();
				isClient = false;
				MostrarMenu(scale, logo);
				//al_draw_textf(font,WHITE,20, 75, ALLEGRO_ALIGN_LEFT,"%f", sin(x)*sin(x));
				if(keys[ALLEGRO_KEY_1]) players = 1;
				if(keys[ALLEGRO_KEY_2]) players = 2;
				if(keys[ALLEGRO_KEY_3]) players = 0;
				if(keys[ALLEGRO_KEY_4]){
					players = 2;
					isClient=true;
					load_screen[CONN] = true;
					gScreen = CONN;
				}
				if(keys[ALLEGRO_KEY_C] && 0){
				  //CAMBIAR RESOLUCION, bad performance
					scale = scale == 1 ? 2 : 1;
					al_destroy_display(display);
					al_destroy_font(font);
					player[1]->SetX(scale * (DEF_W - GROSOR / 2));
					font = al_load_ttf_font("resources/font.ttf", scale * 9, 0) ;
					buffer = al_create_bitmap(scale * resX, scale * resY);
					gScreen = MENU;
				  
				}
				if(keys[ALLEGRO_KEY_ESCAPE]){
					gScreen = 0;
					finish = true;
				}
				else if(keys[ALLEGRO_KEY_1] || keys[ALLEGRO_KEY_2] || keys[ALLEGRO_KEY_3]) {
					gScreen = GAME;
					load_screen[GAME] = true;
					paused = false;
				}
			   
			}
			
			if(input.active)
				input.Draw(30, 60);
			
			al_set_target_backbuffer(display);
			al_clear_to_color(al_map_rgb(0, 0, 0));
			al_draw_scaled_bitmap(buffer, 0, 0, screenWidth, screenHeight, scaleX, scaleY, scaleW, scaleH, 0);
			
			al_wait_for_vsync();
			al_flip_display();
		}

	} while(!finish);
	//[END]---------------BUCLE PRINCIPAL---------------

	printf("Finalizado por el usuario.\n");
	al_rest(0.5);
	kbth.interrupt();
	al_uninstall_audio();
	al_destroy_display(display);
	al_destroy_timer(timer);
	al_destroy_event_queue(event_queue);

	return 0;
}

