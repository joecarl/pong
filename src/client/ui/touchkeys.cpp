
#include "touchkeys.h"
#include "../hengine.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <vector>

using namespace std;

Button::Button(TouchKeys* t_keys, unsigned int keycode, string txt):
	id(-1),
	x(0), 
	y(0), 
	w(20), 
	h(20), 
	keycode(keycode), 
	text(txt),
	pressed(false),
	touch_keys(t_keys)
{

}

void Button::set_dimensions(int x, int y, int w, int h) {
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
}

void Button::set_pressed(int new_touch_id) {
	
	this->id = new_touch_id;
	this->pressed = true;

}

void Button::set_released() {
	
	this->pressed = false;

}

int Button::get_id() {

	return this->id;

}

int Button::get_keycode() {

	return this->keycode;

}

bool Button::is_pressed() {

	return this->pressed;

}

bool Button::in_area(int px, int py) {

	return
		x < px && px < x + w &&
		y < py && py < y + h 
	;

}

void Button::draw() {

	if (text.empty()) {
		return;
	}

	al_draw_rectangle(x + 1, y, x + w, y + h - 1, al_map_rgb(35, 35, 35), 1);
	
	ALLEGRO_COLOR bgcolor;

	if (pressed) {
		bgcolor = al_map_rgb(200, 125, 155);
	} else {
		bgcolor = al_map_rgb(50, 50, 50);
	}

	al_draw_filled_rectangle(x + 1, y + 1, x + w - 1, y + h - 1, bgcolor);

	al_draw_text(
		this->touch_keys->get_engine()->get_font(), 
		al_map_rgb(255, 255, 255), 
		x + w / 2, 
		y + h / 2 - 8, 
		ALLEGRO_ALIGN_CENTER,
		text.c_str()
	);

}


//-------------------------------- TouchKeys ----------------------------------

TouchKeys::TouchKeys(HGameEngine* engine):
engine(engine) {

}


void TouchKeys::clear_buttons() {

	vector<Button>().swap(buttons);

}


Button* TouchKeys::get_pressed_btn_by_id(int id) {
	
	for (auto &btn: buttons) {
		this->engine->debug_txt += " [btn" + to_string(btn.get_id()) + "]" + (btn.is_pressed() ? "P " : "NP ");
		if (btn.is_pressed() && btn.get_id() == id) {
			return &btn;
		}
	}
	
	return nullptr;

}


void TouchKeys::draw() {
	
	for (auto& btn: buttons) {
		btn.draw();
	}

}


void TouchKeys::add_button(unsigned int keycode, string txt) {
	
	Button btn(this, keycode, txt);

	buttons.push_back(std::move(btn));

}

/*
void TouchKeys::fit_buttons(unsigned int side, unsigned int size) {

	this->side = side;
	this->size = size;

	this->re_arrange();

}

void TouchKeys::re_arrange() {

	auto& allegro_hnd = this->engine->get_allegro_hnd();

	int window_w = allegro_hnd.get_window_width() / allegro_hnd.get_scaled();
	int window_h = allegro_hnd.get_window_height() / allegro_hnd.get_scaled();

	unsigned int width, height;

	if (side == FIT_BOTTOM || side == FIT_TOP || side == FIT_HORIZONTAL) {
		width = window_w / buttons.size();
		height = side == FIT_HORIZONTAL ? window_h : window_h * size / 100;
	}
	else if (side == FIT_LEFT || side == FIT_RIGHT || side == FIT_VERTICAL) {
		height = window_h / buttons.size();
		width = side == FIT_VERTICAL ? window_w : window_w * size / 100;
	}

	unsigned int i = 0;

	for (auto &btn: buttons) {

		if (side == FIT_TOP || side == FIT_HORIZONTAL) {
			btn.set_dimensions(i * width, 0, width, height);
		}
		else if (side == FIT_RIGHT) {
			btn.set_dimensions(window_w - width, i * height, width, height);
		}
		else if (side == FIT_BOTTOM) {
			btn.set_dimensions(i * width, window_h - height, width, height);
		}
		else if (side == FIT_LEFT || side == FIT_VERTICAL) {
			btn.set_dimensions(0, i * height, width, height);
		}

		i++;
	}

}
*/

void TouchKeys::layout_buttons(vector<TouchKeysRow>&& layout) {
	
	this->layout = layout;

	this->re_arrange();

}

void TouchKeys::fit_buttons(TouchKeysFit side, uint16_t size) {

	if (side == FIT_BOTTOM || side == FIT_TOP || side == FIT_HORIZONTAL) {

		TouchKeysRow empty_row = {
			.height = static_cast<uint16_t>(100 - size),
			.flex = true,
			.cells = {}
		};

		vector<TouchKeysCell> cells;
		auto count = buttons.size();
		for (uint8_t i = 0; i < count; i++) {
			cells.push_back({
				.width = 1,
				.flex = true
			});
		}

		TouchKeysRow row = {
			.height = size,
			.flex = true,
			.cells = cells
		};

		if (side == FIT_BOTTOM) this->layout_buttons({ empty_row, row });
		if (side == FIT_TOP) this->layout_buttons({ row, empty_row });
		if (side == FIT_HORIZONTAL) this->layout_buttons({ row });
	}
	/*
	else if (side == FIT_LEFT || side == FIT_RIGHT || side == FIT_VERTICAL) {

		
	}
	*/

}

void TouchKeys::arrange_row(TouchKeysRow& row, uint16_t y, uint16_t height, uint16_t window_w) {

	uint16_t flex_w = window_w;
	uint8_t flex_w_divisions = 0;

	for (auto& cell: row.cells) {

		if (!cell.flex) {
			flex_w -= cell.width;
		} else {
			flex_w_divisions += cell.width;
		}

	}

	uint16_t x = 0;
	float remainder = 0;
	for (auto& cell: row.cells) {

		uint16_t width;

		if (cell.flex) {
			const float flex_size = ((float)cell.width / (float)flex_w_divisions) * (float) flex_w;
			width = flex_size;
			remainder += flex_size - width;
			if (remainder >= 1.0) {
				width += 1;
				remainder -= 1.0;
			}
		} else {
			width = cell.width;
		}

		if (this->_arrange_i >= this->buttons.size()) {
			return;
		}

		auto& btn = this->buttons[this->_arrange_i];

		btn.set_dimensions(x, y, width, height);

		x += width;
		this->_arrange_i++;
	}

}

void TouchKeys::re_arrange() {

	auto& allegro_hnd = this->engine->get_allegro_hnd();

	int window_w = allegro_hnd.get_window_width() / allegro_hnd.get_scaled();
	int window_h = allegro_hnd.get_window_height() / allegro_hnd.get_scaled();

	uint16_t flex_h = window_h;
	uint8_t flex_h_divisions = 0;

	for (auto& row: this->layout) {

		if (!row.flex) {
			flex_h -= row.height;
		} else {
			flex_h_divisions += row.height;
		}

	}

	this->_arrange_i = 0;
	uint16_t y = 0;

	for (auto& row: this->layout) {

		uint16_t height = row.flex ? ((float)row.height / (float)flex_h_divisions) * flex_h : row.height;

		this->arrange_row(row, y, height, window_w);

		y += height;
	}

}


void TouchKeys::redefine_touch_event(ALLEGRO_EVENT &evt) {

	switch (evt.type) {

		case ALLEGRO_EVENT_TOUCH_BEGIN:
			{
				int touch_id = evt.touch.id;
				this->engine->debug_txt = "ID BEGIN: " + to_string(touch_id);
				//auto mapped_pt = this->engine->allegro_hnd->get_mapped_coordinates(evt.touch.x, evt.touch.y);
				float scaled = this->engine->get_allegro_hnd().get_scaled();

				for (auto& btn: buttons) {

					//if (btn.in_area(mapped_pt.x, mapped_pt.y)) {
					if (btn.in_area(evt.touch.x / scaled, evt.touch.y / scaled)) {
						evt.type = ALLEGRO_EVENT_KEY_DOWN;
						evt.keyboard.keycode = btn.get_keycode();
						btn.set_pressed(touch_id);
					}
					//ALLEGRO_DEBUG("touch %i begin", event.touch.id);
				}
			}
			break;

		case ALLEGRO_EVENT_TOUCH_END:
			{
				this->engine->debug_txt = "ID END: " + to_string(evt.touch.id);
				Button *btn = this->get_pressed_btn_by_id(evt.touch.id);

				if (btn != nullptr) {
					evt.type = ALLEGRO_EVENT_KEY_UP;
					evt.keyboard.keycode = btn->get_keycode();
					btn->set_released();
				} else {
					this->engine->debug_txt += " not found";
				}
			}
			break;

		case ALLEGRO_EVENT_TOUCH_MOVE:
			//ALLEGRO_DEBUG("touch %i move: %fx%f", event.touch.id, event.touch.x, event.touch.y);
			break;

	}

}

