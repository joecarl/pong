
#include "touchkeys.h"
#include "hengine.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <vector>

using namespace std;

Button::Button(TouchKeys* tKeys, unsigned int keycode, string txt):
	id(-1),
	x(0), 
	y(0), 
	w(20), 
	h(20), 
	keycode(keycode), 
	text(txt),
	pressed(false),
	touch_keys(tKeys)
{

}

void Button::set_dimensions(int x, int y, int w, int h) {
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
}

void Button::set_pressed(int newTouchID) {
	
	this->id = newTouchID;
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

	al_draw_rectangle(x + 1, y, x + w, y + h - 1, al_map_rgb(255, 255, 255), 1);
	
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

	unsigned int width, height;

	if (side == FIT_BOTTOM || side == FIT_TOP || side == FIT_HORIZONTAL) {
		width = this->engine->res_x / buttons.size();
		height = side == FIT_HORIZONTAL ? this->engine->res_y : size;
	}
	else if (side == FIT_LEFT || side == FIT_RIGHT || side == FIT_VERTICAL) {
		height = this->engine->res_y / buttons.size();
		width = side == FIT_VERTICAL ? this->engine->res_x : size;
	}

	unsigned int i = 0;

	for (auto &btn: buttons) {

		if (side == FIT_TOP || side == FIT_HORIZONTAL) {
			btn.set_dimensions(i * width, 0, width, height);
		}
		else if (side == FIT_RIGHT) {
			btn.set_dimensions(this->engine->res_x - width, i * height, width, height);
		}
		else if (side == FIT_BOTTOM) {
			btn.set_dimensions(i * width, this->engine->res_y - height, width, height);
		}
		else if (side == FIT_LEFT || side == FIT_VERTICAL) {
			btn.set_dimensions(0, i * height, width, height);
		}

		i++;
	}

}
*/


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

void TouchKeys::redefine_touch_event(ALLEGRO_EVENT &evt) {

	switch (evt.type) {

		case ALLEGRO_EVENT_TOUCH_BEGIN:
			{
				int touch_id = evt.touch.id;
				this->engine->debug_txt = "ID BEGIN: " + to_string(touch_id);
				//auto mappedPt = this->engine->allegro_hnd->get_mapped_coordinates(evt.touch.x, evt.touch.y);
				float scaled = this->engine->get_allegro_hnd().get_scaled();

				for (auto& btn: buttons) {

					//if (btn.in_area(mappedPt.x, mappedPt.y)) {
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

