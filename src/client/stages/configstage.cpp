#include "configstage.h"
#include "../mediatools.h"
#include "../ui/input.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <string>

using namespace std;


ConfigStage::ConfigStage(HGameEngine* _engine) : Stage(_engine) {

	auto playername_input = this->engine->create_text_input();
	auto serverhostname_input = this->engine->create_text_input();
	// auto windowed_input = this->engine->create_checkbox_input();

	config_params = {
		{ .key = "playerName", .label = "Player Name", .input = playername_input },
		{ .key = "serverHostname", .label = "Server", .input = serverhostname_input },
		//{ .key = "windowed", .label = "Run windowed", .input = windowed_input },
	};

}

void ConfigStage::on_enter_stage() {

	this->welcome_view = !this->engine->get_cfg().contains("playerName");

	auto& touch_keys = this->engine->get_touch_keys();
	auto playername_input = this->config_params[0].input;

	this->saved = false;

	inp_index = 0;
	playername_input->focus();

	touch_keys.clear_buttons();
	if (!this->welcome_view) {
		touch_keys.add_button(ALLEGRO_KEY_UP, "__arrow_up");
		touch_keys.add_button(ALLEGRO_KEY_DOWN, "__arrow_down");
		touch_keys.add_button(ALLEGRO_KEY_LEFT, "__arrow_left");
		touch_keys.add_button(ALLEGRO_KEY_RIGHT, "__arrow_right");
	}
	touch_keys.add_button(ALLEGRO_KEY_ENTER, "Ok");
	touch_keys.add_button(ALLEGRO_KEY_ESCAPE, "Esc");
	touch_keys.fit_buttons(FIT_BOTTOM, 10);

	auto& cfg = this->engine->get_cfg();
	for (auto& param: config_params) {

		if (!cfg.contains(param.key)) {
			continue;
		}

		param.input->set_from_json_value(cfg[param.key]);

	} 

}

bool ConfigStage::saved_modified_inputs() {

	for (auto& param: config_params) {

		// if (modified)
		auto val = param.input->get_json_value();
		this->engine->set_cfg_param(param.key, val);

		//if (error de validacion) return false;

	}

	this->saved = true;

	return true;

}


void ConfigStage::on_event(ALLEGRO_EVENT event) {

	if (event.type == ALLEGRO_EVENT_KEY_DOWN) {

		int keycode = event.keyboard.keycode;

		if (this->welcome_view) {

			if (keycode == ALLEGRO_KEY_ENTER) {

				if (!this->saved) {

					if (this->saved_modified_inputs()) {
						this->engine->set_active_input(nullptr);
					}

				} else {

					al_rest(0.2);
					this->engine->set_stage(MENU);

				}
			}
			return;

		}

		if (keycode == ALLEGRO_KEY_UP || keycode == ALLEGRO_KEY_DOWN) {

			if (keycode == ALLEGRO_KEY_UP && inp_index > 0) {
				inp_index--;
			}

			if (keycode == ALLEGRO_KEY_DOWN && inp_index < this->config_params.size() - 1) {
				inp_index++;
			}

			this->config_params[inp_index].input->focus();

		} else if (keycode == ALLEGRO_KEY_ESCAPE) {//ESC (SALIR)

			this->engine->set_stage(MENU);

		} else if (keycode == ALLEGRO_KEY_ENTER) {

			if (this->saved_modified_inputs()) {	
				this->engine->set_stage(MENU);
			}

		}

	}

}


/*
void ConfigStage::on_tick() {

}
*/


void ConfigStage::draw_welcome_view() {
	
	ALLEGRO_FONT* font = this->engine->get_font();
	
	auto playername_input = static_cast<TextInput*>(this->config_params[0].input);

	if (playername_input->is_focused()) {

		al_draw_text(font, WHITE, 30, 30, ALLEGRO_ALIGN_LEFT, "Choose a name:");

		playername_input->draw(30, 60);

	} else {
		
		auto& cfg = this->engine->get_cfg();

		string player_name = cfg.contains("playerName") ? cfg["playerName"].as_string().c_str() : "-" ;
		string msg = "Welcome " + player_name + "!";

		al_draw_text(font, WHITE, 30, 30, ALLEGRO_ALIGN_LEFT, msg.c_str());
		al_draw_text(font, WHITE, 30, 50, ALLEGRO_ALIGN_LEFT, "Press enter to continue");

	}

}


void ConfigStage::draw_config_view() {

	// press F1 to rever to default ??

	static float frame = 0;
	
	ALLEGRO_FONT* font = this->engine->get_font();

	const int line_height = 20;

	int y = 50;
	int marker_y = y + inp_index * line_height + 7;

	const float x_shift = 3.0 * sin(frame / 6.0);

	al_draw_filled_circle(20 + x_shift, marker_y, 3, RED);
	al_draw_filled_circle(320 - 20 - x_shift, marker_y, 3, RED);

	for (auto& param: config_params) {

		al_draw_text(font, WHITE, 35, y, ALLEGRO_ALIGN_LEFT, param.label.c_str());

		if (param.input->type == INPUT_TYPE_TEXT) {
			TextInput* inp = static_cast<TextInput*>(param.input);
			inp->draw(160, y);
		}
		//param.input->draw(150, y);

		y += line_height;

	}

	frame++;

}


void ConfigStage::draw() {

	if (this->welcome_view) {

		this->draw_welcome_view();

	} else {

		this->draw_config_view();

	}
	
}
