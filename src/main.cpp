
#include <iostream>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>

#include "hengine.h"

#define TICKS_PER_SECOND 60.0

using namespace std;

/*
void timer_task(int *ms){
	while (1) {
		//usleep(1000);
		boost::this_thread::sleep(boost::posix_time::milliseconds(1));
		(*ms)++;
	}
}
*/

//------------------------------------------------------------------------------
//--------------------------- PROGRAMA PRINCIPAL -------------------------------
//------------------------------------------------------------------------------

int main(int argc, char **argv){
	
	al_init_image_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_primitives_addon();
	
	if(!al_init()) {
		cerr << "failed to initialize allegro!" << endl;
		return -1;
	}
	cout << "allegro initialized" << endl;

	//al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
	al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_REQUIRE);

	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_DISPLAY_MODE disp_data;
	ALLEGRO_BITMAP *buffer;
	
	al_get_display_mode(al_get_num_display_modes() - 1, &disp_data);

	HGameEngine *gameEngine = new HGameEngine();
	
	gameEngine->setStage(MENU);

	int windowWidth = disp_data.width;
	int windowHeight = disp_data.height;
	int screenWidth = gameEngine->scale * gameEngine->resX;
	int screenHeight = gameEngine->scale * gameEngine->resY;

	display = al_create_display(windowWidth, windowHeight);
	if(!display){
		cerr << "failed to create display!" << endl;
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
	
	cout << "display created" << endl;
	/*
	for(int i=0; i< al_get_num_display_modes(); i++){
		al_get_display_mode(i, &disp_data);
		cout << "Resolution: " << disp_data.width << "x" << disp_data.height << endl;
	}
	*/

	if (!al_install_audio()) {
		cerr << "Could not init sound!" << endl;
		return -1; 
	}
	if (!al_reserve_samples(1)) {
		cerr << "Could not reserve samples!" << endl;
		return -1; 
	}
	
	//al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
	
	cout << "elements loaded" << endl;

	al_install_keyboard();
	
	ALLEGRO_TIMER* timer = al_create_timer(1.0 / TICKS_PER_SECOND);
	ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	al_start_timer(timer);
	al_hide_mouse_cursor(display);

	cout << "settings done" << endl;

	ALLEGRO_EVENT event;
	
	//int milliseconds = 0;
	//boost::thread th_tim(timer_task, &milliseconds);
	
	do{

		al_wait_for_event(event_queue, &event);
		if(
			event.type == ALLEGRO_EVENT_KEY_CHAR || 
			event.type == ALLEGRO_EVENT_KEY_DOWN ||
			event.type == ALLEGRO_EVENT_KEY_UP
		){
			gameEngine->onEvent(event);
		}

		else if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE){
			break;
		}

		else if(event.type == ALLEGRO_EVENT_TIMER){

			gameEngine->runTick();
			
			al_set_target_bitmap(buffer);
			al_clear_to_color(al_map_rgb(0, 0, 0));
			
			gameEngine->draw();
			
			al_set_target_backbuffer(display);
			al_clear_to_color(al_map_rgb(0, 0, 0));
			al_draw_scaled_bitmap(buffer, 0, 0, screenWidth, screenHeight, scaleX, scaleY, scaleW, scaleH, 0);
			
			al_wait_for_vsync();
			al_flip_display();
		}

	} while(!gameEngine->finish);

	cout << "Finalizado por el usuario." << endl;
	al_rest(0.5);
	al_uninstall_audio();
	al_destroy_display(display);
	al_destroy_timer(timer);
	al_destroy_event_queue(event_queue);

	return 0;

}

