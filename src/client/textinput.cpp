#include "textinput.h"
#include "mediatools.h"

#include <allegro5/allegro_primitives.h>


TextInput::TextInput(ALLEGRO_FONT* fuente) {
	
	font = fuente;//al_load_ttf_font("font.ttf", 2*9, 0);
	reset();

}

void TextInput::reset() {

	edittext = "";
	iter = edittext.begin();
	caret = 0;
	caret_time = 0;
	insert = true;
	active = false;

}

void  TextInput::process_key(wchar_t ASCII, int control_key) {

	if (ASCII >= 32 && ASCII <= 126) {
		
		// add the new char, inserting or replacing as need be
		if (insert || iter == edittext.end())
			iter = edittext.insert(iter, ASCII);
		else
			edittext.replace(caret, 1, 1, ASCII);
		
		// increment both the caret and the iterator
		caret++;
		iter++;

	}

	// some other, "special" key was pressed; handle it here
	else switch (control_key) {

		case ALLEGRO_KEY_DELETE://delete
			if (iter != edittext.end()) iter = edittext.erase(iter);
			break;
			
		case ALLEGRO_KEY_BACKSPACE://backspace
			if (iter != edittext.begin())
			{
				caret--;
				iter--;
				iter = edittext.erase(iter);
			}
			break;
			
		case ALLEGRO_KEY_RIGHT:
			if (iter != edittext.end())   caret++, iter++;
			break;
			
		case ALLEGRO_KEY_LEFT:
			if (iter != edittext.begin()) caret--, iter--;
			break;
			
		case ALLEGRO_KEY_INSERT:
			insert = !insert;
			break;
			
		default:
			break;
	}

}

void  TextInput::draw(int x, int y) {

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

void TextInput::start() {

	reset();
	active = true;

}

void  TextInput::finish() {

	active = false;

}

std::string TextInput::get_value() {

	return edittext;

}
