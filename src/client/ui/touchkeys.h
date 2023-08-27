#ifndef TOUCHKEYS_H
#define TOUCHKEYS_H

#include <allegro5/allegro.h>
#include <string>
#include <vector>

class TouchKeys;

class HGameEngine;

enum TouchKeysFit {
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


typedef struct {

	uint16_t width;
	bool flex;
	//bool empty;

} TouchKeysCell;

typedef struct {

	/**
	 * Determines the row height. If flex is true this value will be used to
	 * calculate the relative size. If flex is false this value will be used as
	 * an exact amount of pixels.
	 */
	uint16_t height;

	/**
	 * The minimum relative size (this value is only used when flex is false)
	 * Set to 0 to disable
	 */
	uint16_t min_flex_height = 0;

	/**
	 * The maximum relative size (this value is only used when flex is false)
	 * Set to 0 to disable
	 */
	uint16_t max_flex_height = 0;

	/**
	 * Wether to use flex sizing or not
	 */
	bool flex;

	/**
	 * Container for the cells in the wor
	 */
	std::vector<TouchKeysCell> cells;

	/**
	 * Used for internal calculations. Don't touch
	 */
	uint16_t calculated_height = 0;

} TouchKeysRow;


class TouchKeys {

	std::vector<Button> buttons;

	Button* get_pressed_btn_by_id(int id);

	HGameEngine* engine;

	//unsigned int side; 
	//unsigned int size;

	std::vector<TouchKeysRow> layout;

	
	uint16_t _arrange_i = 0;

	void arrange_row(TouchKeysRow& row, uint16_t y, uint16_t height, uint16_t window_w);

public:

	TouchKeys(HGameEngine* engine);

	void clear_buttons();

	void add_button(unsigned int keycode, std::string txt);

	void layout_buttons(std::vector<TouchKeysRow>&& layout);

	void fit_buttons(TouchKeysFit side, uint16_t size = 1);

	void re_arrange();

	void redefine_touch_event(ALLEGRO_EVENT &evt);

	void draw();

	HGameEngine* get_engine() {
		return this->engine;
	}

};

#endif
