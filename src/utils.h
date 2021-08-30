#ifndef UTILSH
#define UTILSH

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <string>

#define WHITE al_map_rgb(255, 255, 255)
using namespace std;

const int Do = 1, DoSos = 2, Re = 3, ReSos = 4, Mi = 5, Fa = 6, FaSos = 7,
	  Sol = 8, SolSos = 9, La = 10, LaSos = 11, Si = 12;//
/*const Do=522, DoSos=554, Re=588, ReSos=622, Mi=660, Fa=698, FaSos=740,
	  Sol=784, SolSos=830, La=880, LaSos=932, Si=988*/

float random_ex(int max, float min = 0.0);

string GetWaitString();

void  PlaySound(int nota, float time, int octava = 4);

//void  ShowKeyBoardMatrix();

void PlayAudio(float volumen = 1.0, ALLEGRO_PLAYMODE mode = ALLEGRO_PLAYMODE_ONCE);

void PlayExorcista();

class JC_TEXTINPUT{
	
	ALLEGRO_FONT* font;
	string  edittext;	   // an empty string for editting
	string::iterator iter; // string iterator
	int	caret;		// tracks the text caret
	bool insert;	// true of should text be inserted
	char* destino;
	int caret_time;
	
public:

	bool active;

	JC_TEXTINPUT(ALLEGRO_FONT* fuente);

	void Reset();
	
	void ProcessKey(wchar_t ASCII, int control_key);

	void Draw(int x, int y);

	void Start(char* dest);
	
	void Finish();
	
	const char* GetValue();

};

#endif