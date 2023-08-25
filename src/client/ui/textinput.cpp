#include "textinput.h"
#include "../mediatools.h"
#include "../hengine.h"

#include <allegro5/allegro_primitives.h>


TextInput::TextInput(HGameEngine* _engine) :
	engine(_engine)
{
	this->type = INPUT_TYPE_TEXT;
	//font = fuente;//al_load_ttf_font("font.ttf", 2*9, 0);
	reset();

}

void TextInput::focus() {
	
	this->engine->set_active_input(this);
	
}


bool TextInput::is_focused() {

	return this->engine->get_active_input() == this;

}


void TextInput::blur() {

	if (this->is_focused()) {
		this->engine->set_active_input(nullptr);
	} 

}

void TextInput::reset() {

	edittext = "";
	iter = edittext.begin();
	caret = 0;
	caret_time = 0;
	insert = true;

}

void TextInput::process_key(wchar_t ASCII, int control_key) {

	/*
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
	*/
	char ascii = 0;

	switch (control_key) {
		case ALLEGRO_KEY_FULLSTOP:
			ascii = '.';
			break;
		case ALLEGRO_KEY_SPACE:
			ascii = ' ';
			break;
	}
	
	if (control_key >= ALLEGRO_KEY_A && control_key <= ALLEGRO_KEY_Z) {
		ascii = control_key - ALLEGRO_KEY_A + 'A';
	}
	if (control_key >= ALLEGRO_KEY_0 && control_key <= ALLEGRO_KEY_9) {
		ascii = control_key - ALLEGRO_KEY_0 + '0';
	}

	if (ascii >= 32 && ascii <= 126) {
		
		// add the new char, inserting or replacing as need be
		if (insert || iter == edittext.end())
			iter = edittext.insert(iter, ascii);
		else
			edittext.replace(caret, 1, 1, ascii);
		
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

void TextInput::draw(float x, float y) {

	ALLEGRO_FONT* font = this->engine->get_font();

	al_draw_text(font, WHITE, x, y, ALLEGRO_ALIGN_LEFT, edittext.c_str());

	if (!this->is_focused()) {
		return;
	}

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


std::string TextInput::get_value() {

	return edittext;

}


void TextInput::set_from_json_value(boost::json::value& val) {
	
	if (val.is_string()) {
		this->edittext = val.get_string().c_str();
	}

}


boost::json::value TextInput::get_json_value() {

	return boost::json::value(this->get_value());

}
