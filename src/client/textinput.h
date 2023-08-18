#ifndef TEXTINPUT_H
#define TEXTINPUT_H

#include <allegro5/allegro_font.h>
#include <string>

class TextInput {
	
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

	TextInput(ALLEGRO_FONT* fuente);

	void reset();
	
	void process_key(wchar_t ASCII, int control_key);

	void draw(int x, int y);

	void start();
	
	void finish();
	
	std::string get_value();

};

#endif