#include "tutorialstage.h"
#include "../pongclient.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <string>


void TutorialStage::on_enter_stage() {

	//this->thumb_press = dp::client::load_bitmap(THUMB_PRESS);
	//this->thumb_release = dp::client::load_bitmap(THUMB_RELEASE);
	this->thumb_press = this->engine->load_bitmap_resource(THUMB_PRESS);
	this->thumb_release = this->engine->load_bitmap_resource(THUMB_RELEASE);
	this->engine->set_cfg_param("tutorialCompleted", true);

	auto& touch_keys = this->engine->get_touch_keys();

	touch_keys.clear_buttons();

	touch_keys.add_button(ALLEGRO_KEY_ENTER, "Ok");

	touch_keys.fit_buttons(dp::client::ui::FIT_BOTTOM, 10);

}

void TutorialStage::on_event(ALLEGRO_EVENT event) {

	int keycode = event.keyboard.keycode;

	if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
	
		if (keycode == ALLEGRO_KEY_ENTER) {
			this->engine->set_stage(GAME);
		}

	}

}


void TutorialStage::draw_mode_one() {

	float scale = this->engine->get_scale();
	static float y = 0;
	static uint32_t frame = 0;
	static bool pressed_left = false;
	static bool pressed_right = false;
	static std::string text = "";
	static const int blink_mod = 40;

	al_draw_rounded_rectangle(
		scale * 50, 
		scale * 40, 
		scale * (50 + 220), 
		scale * (40 + 100), 
		scale * 10,
		scale * 10,
		WHITE,
		0
	);

	const auto red_color = al_map_rgb(255, 150, 150);
	const float vx[] = {
		50 + 60, 65 + y - 5,
		50 + 55, 65 + y,
		50 + 60, 65 + y + 5,
		50 + 60, 65 + y + 1,
		50 + 75, 65 + y + 1,
		50 + 75, 65 + y - 1, 
		50 + 60, 65 + y - 1
	};
	if (frame % blink_mod > blink_mod / 2) {
		al_draw_filled_polygon(vx, 7, red_color);
	}

	al_draw_rectangle(
		scale * (50 + 50), 
		scale * (40 + 20 + y), 
		scale * (50 + 50 + 1), 
		scale * (40 + 20 + 8 + y), 
		WHITE,
		1.0
	);

	al_draw_line(160, 50, 160, 130, WHITE, 1);

	al_draw_line(
		160 - 65, 
		50, 
		160 + 65, 
		50, 
		WHITE, 
		1
	);

	al_draw_line(
		160 - 65, 
		130, 
		160 + 65, 
		130, 
		WHITE, 
		1
	);

	al_draw_rectangle(
		scale * (50 + 170), 
		scale * (40 + 70), 
		scale * (50 + 170 + 1), 
		scale * (40 + 70 + 8), 
		WHITE,
		1.0
	);


	if (pressed_left) {
		al_draw_bitmap(this->thumb_press, 25, 70, 0);
		y += 0.8;
	} else {
		al_draw_bitmap(this->thumb_release, 25, 70, 0);
	}

	if (pressed_right) {
		al_draw_bitmap(this->thumb_press, 225, 60, ALLEGRO_FLIP_HORIZONTAL);
		y -= 0.8;
	} else {
		al_draw_bitmap(this->thumb_release, 225, 60, ALLEGRO_FLIP_HORIZONTAL);
	}


	al_draw_text(
		this->engine->get_font(), 
		red_color,
		scale * 160, 
		scale * 5, 
		ALLEGRO_ALIGN_CENTER, 
		text.c_str()
	);

	if (frame == 1) {
		text = "PRESS ON THE LEFT SIDE TO MOVE DOWN";
	} else if (frame == 60) {
		pressed_left = true;
	} else if (frame == 100) {
		pressed_left = false;
	} else if (frame == 140) {
		pressed_left = true;
	} else if (frame == 160) {
		pressed_left = false;
	} else if (frame == 200) {
		text = "PRESS ON THE RIGHT SIDE TO MOVE UP";
		pressed_right = true;
	} else if (frame == 240) {
		pressed_right = false;
	} else if (frame == 280) {
		pressed_right = true;
	} else if (frame == 300) {
		pressed_right = false;
	} else if (frame == 340) {
		text = "DON'T SLIDE YOUR FINGER!";
	} else if (frame == 500) {
		frame = 0;
	} 

	frame++;

}


void TutorialStage::draw() {

	//if (this->target_stage == )
	this->draw_mode_one();

}


void TutorialStage::draw_mode_two() {


}