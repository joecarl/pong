
#include "touchkeys.h"
#include "hengine.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <vector>

using namespace std;

Button::Button(TouchKeys* tKeys, unsigned int keycode, string txt):
id(-1),
x(0), y(0), 
w(20), h(20), 
keycode(keycode), text(txt),
pressed(false),
touchKeys(tKeys)
{

}

void Button::setDimensions(int x, int y, int w, int h){
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
}

void Button::setPressed(int newTouchID){
	
	this->id = newTouchID;
	this->pressed = true;

}

void Button::setReleased(){
	
	this->pressed = false;

}

int Button::getID(){

	return this->id;

}

int Button::getKeycode(){

	return this->keycode;

}

bool Button::isPressed(){

	return this->pressed;

}

bool Button::inArea(int px, int py){

	return
		x < px && px < x + w &&
		y < py && py < y + h 
	;

}

void Button::draw(){

	if(text.empty()){
		return;
	}

	al_draw_rectangle(x + 1, y, x + w, y + h - 1, al_map_rgb(255, 255, 255), 1);
	
	if(pressed){
		al_draw_filled_rectangle(x + 1, y + 1, x + w, y + h - 1, al_map_rgb(200, 125, 155));
	}

	al_draw_text(
		this->touchKeys->getEngine()->font, 
		al_map_rgb(255, 255, 255), 
		x + w / 2, 
		y + h / 2 - 8, 
		ALLEGRO_ALIGN_CENTER,
		text.c_str()
	);

}


//-------------------------------- TouchKeys ----------------------------------

TouchKeys::TouchKeys(HGameEngine* engine):
engine(engine){

}


void TouchKeys::clearButtons(){

	vector<Button>().swap(buttons);

}


Button* TouchKeys::getPressedBtnByID(int id){
	
	for(auto &btn: buttons){
		this->engine->debugTxt += " [btn" + to_string(btn.getID()) + "]"+(btn.isPressed() ? "P " : "NP ");
		if(btn.isPressed() && btn.getID() == id){
			return &btn;
		}
	}
	
	return nullptr;

}


void TouchKeys::draw(){
	
	for(auto& btn: buttons){
		btn.draw();
	}

}


void TouchKeys::addButton(unsigned int keycode, string txt){
	
	Button btn(this, keycode, txt);

	buttons.push_back(std::move(btn));

}

/*
void TouchKeys::fitButtons(unsigned int side, unsigned int size){

	unsigned int width, height;

	if(side == FIT_BOTTOM || side == FIT_TOP || side == FIT_HORIZONTAL){
		width = this->engine->resX / buttons.size();
		height = side == FIT_HORIZONTAL ? this->engine->resY : size;
	}
	else if(side == FIT_LEFT || side == FIT_RIGHT || side == FIT_VERTICAL){
		height = this->engine->resY / buttons.size();
		width = side == FIT_VERTICAL ? this->engine->resX : size;
	}

	unsigned int i = 0;

	for(auto &btn: buttons){

		if(side == FIT_TOP || side == FIT_HORIZONTAL){
			btn.setDimensions(i * width, 0, width, height);
		}
		else if(side == FIT_RIGHT){
			btn.setDimensions(this->engine->resX - width, i * height, width, height);
		}
		else if(side == FIT_BOTTOM){
			btn.setDimensions(i * width, this->engine->resY - height, width, height);
		}
		else if(side == FIT_LEFT || side == FIT_VERTICAL){
			btn.setDimensions(0, i * height, width, height);
		}

		i++;
	}

}
*/


void TouchKeys::fitButtons(unsigned int side, unsigned int size){

	this->side = side;
	this->size = size;

	this->reArrange();

}

void TouchKeys::reArrange(){

	int windowW = this->engine->allegroHnd->getWindowWidth() / this->engine->allegroHnd->getScaled();
	int windowH = this->engine->allegroHnd->getWindowHeight() / this->engine->allegroHnd->getScaled();

	unsigned int width, height;

	if(side == FIT_BOTTOM || side == FIT_TOP || side == FIT_HORIZONTAL){
		width = windowW / buttons.size();
		height = side == FIT_HORIZONTAL ? windowH : windowH * size / 100;
	}
	else if(side == FIT_LEFT || side == FIT_RIGHT || side == FIT_VERTICAL){
		height = windowH / buttons.size();
		width = side == FIT_VERTICAL ? windowW : windowW * size / 100;
	}

	unsigned int i = 0;

	for(auto &btn: buttons){

		if(side == FIT_TOP || side == FIT_HORIZONTAL){
			btn.setDimensions(i * width, 0, width, height);
		}
		else if(side == FIT_RIGHT){
			btn.setDimensions(windowW - width, i * height, width, height);
		}
		else if(side == FIT_BOTTOM){
			btn.setDimensions(i * width, windowH - height, width, height);
		}
		else if(side == FIT_LEFT || side == FIT_VERTICAL){
			btn.setDimensions(0, i * height, width, height);
		}

		i++;
	}

}

void TouchKeys::redefineTouchEvent(ALLEGRO_EVENT &evt){

	switch (evt.type) {

		case ALLEGRO_EVENT_TOUCH_BEGIN:
			{
				int touchID = evt.touch.id;
				this->engine->debugTxt = "ID BEGIN: " + to_string(touchID);
				//auto mappedPt = this->engine->allegroHnd->getMappedCoordinates(evt.touch.x, evt.touch.y);
				float scaled = this->engine->allegroHnd->getScaled();

				for(auto& btn: buttons){

					//if(btn.inArea(mappedPt.x, mappedPt.y)){
					if(btn.inArea(evt.touch.x / scaled, evt.touch.y / scaled)){
						evt.type = ALLEGRO_EVENT_KEY_DOWN;
						evt.keyboard.keycode = btn.getKeycode();
						btn.setPressed(touchID);
					}
					//ALLEGRO_DEBUG("touch %i begin", event.touch.id);
				}
			}
			break;

		case ALLEGRO_EVENT_TOUCH_END:
			{
				this->engine->debugTxt = "ID END: " + to_string(evt.touch.id);
				Button *btn = this->getPressedBtnByID(evt.touch.id);

				if(btn != nullptr){
					evt.type = ALLEGRO_EVENT_KEY_UP;
					evt.keyboard.keycode = btn->getKeycode();
					btn->setReleased();
				} else {
					this->engine->debugTxt += " not found";
				}
			}
			break;

		case ALLEGRO_EVENT_TOUCH_MOVE:
			//ALLEGRO_DEBUG("touch %i move: %fx%f", event.touch.id, event.touch.x, event.touch.y);
			break;

	}

}

