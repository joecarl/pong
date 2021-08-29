#ifndef FUNCIONESH
#define FUNCIONESH

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
const char* scanf_ex(int x, int y);
void PlayExorcista();
void PlayAudio(float volumen = 1.0, ALLEGRO_PLAYMODE mode = ALLEGRO_PLAYMODE_ONCE);

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
	void Reset(){
		edittext="";
		iter = edittext.begin();
		caret  = 0;
		caret_time=0;
		insert = true;
		active = false;
		destino = NULL;
	}
	JC_TEXTINPUT(ALLEGRO_FONT* fuente){
	   
		font=fuente;//al_load_ttf_font("font.ttf", 2*9, 0);
		Reset();
	}
	
	void ProcessKey(wchar_t ASCII, int control_key)
	{
		if(ASCII >= 32 && ASCII <= 126)
		{
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
		else switch(control_key)
		{
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
	void Draw(int x, int y){
		al_draw_text(font, WHITE, x, y, ALLEGRO_ALIGN_LEFT, edittext.c_str());
		if (caret_time<30) {
			char text_caret[100]; 
			strcpy(text_caret, edittext.c_str());
			text_caret[caret]='\0';
			int length=al_get_text_width(font, text_caret);
			al_draw_line(x+length+2, y, x+length+2, y+2*6, WHITE, 1);
		}
		caret_time++;
		if (caret_time>60) {
			caret_time=0;
		}
	}
	void Start(char* dest){
		Reset();
		destino=dest;
		active=true;
	}
	void Finish(){
		if (destino!=NULL) {
			strcpy(destino, edittext.c_str());
			destino=NULL;
			active=false;
		}
		else
			throw 1;
	} 
	const char* GetValue(){
		return edittext.c_str();
	}
};

#endif