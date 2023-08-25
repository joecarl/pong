#ifndef TEXTINPUT_H
#define TEXTINPUT_H

#include "input.h"

#include <allegro5/allegro_font.h>
#include <string>

class HGameEngine;

class TextInput : public Input {
	
	/**
	 * The font used to draw the text
	 */
	HGameEngine* engine;

	/**
	 * An empty string for editting
	 */
	std::string edittext;
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

	TextInput(HGameEngine* _engine);

	void reset();
	
	void process_key(wchar_t ASCII, int control_key);

	void draw(float x, float y);

	void focus();

	void blur();

	bool is_focused();
	
	void set_from_json_value(boost::json::value& val);

	boost::json::value get_json_value();

	std::string get_value();

};

#endif