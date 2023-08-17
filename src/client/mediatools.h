#ifndef MEDIATOOLS_H
#define MEDIATOOLS_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <string>

#define ALPHA_COLOR al_map_rgb(255, 0, 255)

#define WHITE al_map_rgb(255, 255, 255)

#ifdef __APPLE__
#define RES_DIR "../Resources"
#else //if defined _WIN32 || defined _WIN64
#define RES_DIR "resources"
#endif

const int Do = 1, DoSos = 2, Re = 3, ReSos = 4, Mi = 5, Fa = 6, FaSos = 7,
	  Sol = 8, SolSos = 9, La = 10, LaSos = 11, Si = 12;//
/*const Do=522, DoSos=554, Re=588, ReSos=622, Mi=660, Fa=698, FaSos=740,
	  Sol=784, SolSos=830, La=880, LaSos=932, Si=988*/


void play_sound(int nota, float time, int octava = 4);

//void  ShowKeyBoardMatrix();

ALLEGRO_BITMAP* load_bitmap(const std::string& filename);

void play_audio(float volumen = 1.0, ALLEGRO_PLAYMODE mode = ALLEGRO_PLAYMODE_ONCE);

void play_exorcista();

class JC_TEXTINPUT {
	
	/**
	 * The font used to draw the text
	 */
	ALLEGRO_FONT* font;

	/**
	 * An empty string for editting
	 */
	std::string  edittext;
	/**
	 * String iterator
	 */
	std::string::iterator iter;

	/**
	 * Tracks the text caret
	 */
	int	caret;

	/**
	 * True if should text be inserted
	 */
	bool insert;
	
	/**
	 * Caret blinking control
	 */
	int caret_time;
	
public:

	bool active;

	JC_TEXTINPUT(ALLEGRO_FONT* fuente);

	void reset();
	
	void process_key(wchar_t ASCII, int control_key);

	void draw(int x, int y);

	void start();
	
	void finish();
	
	std::string get_value();

};

#endif
