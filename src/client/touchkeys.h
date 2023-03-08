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

	TouchKeys* touchKeys;

public:

	Button(TouchKeys * tKeys, unsigned int keycode, std::string txt);

	void setDimensions(int x, int y, int w, int h);

	void setPressed(int newTouchID);
	
	void setReleased();

	int getID();

	int getKeycode();

	bool isPressed();

	bool inArea(int px, int py);

	void draw();

};


class TouchKeys {

	std::vector<Button> buttons;

	Button* getPressedBtnByID(int id);

	HGameEngine* engine;

	
	unsigned int side; 
	unsigned int size;

public:

	TouchKeys(HGameEngine* engine);

	void clearButtons();

	void addButton(unsigned int keycode, std::string txt);

	void fitButtons(unsigned int side, unsigned int size = 0);

	void reArrange();

	void redefineTouchEvent(ALLEGRO_EVENT &evt);

	void draw();

	HGameEngine* getEngine() {
		return this->engine;
	}

};

#endif
