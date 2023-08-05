#ifndef TOUCHKEYSH
#define TOUCHKEYSH

#include <allegro5/allegro.h>
#include <string>
#include <vector>

class TouchKeys;

class HGameEngine;

enum {
	FIT_TOP = 1,
	FIT_RIGHT,
	FIT_BOTTOM,
	FIT_LEFT,
	FIT_VERTICAL,
	FIT_HORIZONTAL
};

class Button {

	int id;

	int x, y, w, h;

	unsigned int keycode;

	std::string text;

	bool pressed;

	TouchKeys* touch_keys;

public:

	Button(TouchKeys* t_keys, unsigned int keycode, std::string txt);

	void set_dimensions(int x, int y, int w, int h);

	void set_pressed(int new_touch_id);
	
	void set_released();

	int get_id();

	int get_keycode();

	bool is_pressed();

	bool in_area(int px, int py);

	void draw();

};


class TouchKeys {

	std::vector<Button> buttons;

	Button* get_pressed_btn_by_id(int id);

	HGameEngine* engine;

	
	unsigned int side; 
	unsigned int size;

public:

	TouchKeys(HGameEngine* engine);

	void clear_buttons();

	void add_button(unsigned int keycode, std::string txt);

	void fit_buttons(unsigned int side, unsigned int size = 0);

	void re_arrange();

	void redefine_touch_event(ALLEGRO_EVENT &evt);

	void draw();

	HGameEngine* get_engine() {
		return this->engine;
	}

};

#endif
