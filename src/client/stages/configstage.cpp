#include "configstage.h"
#include "../pongclient.h"
#include <dp/client/ui/input.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <string>

using dp::client::BaseClient;
using dp::client::ui::TextInput;
using std::string;


ConfigStage::ConfigStage(BaseClient* _engine) : Stage(_engine) {

	auto playername_input = this->engine->create_text_input();
	playername_input->min_chars = 3;
	playername_input->max_chars = 12;

	auto serverhostname_input = this->engine->create_text_input();
	serverhostname_input->min_chars = 3;
	serverhostname_input->max_chars = 19;

	auto showfps_input = this->engine->create_boolean_input();
	auto showping_input = this->engine->create_boolean_input();
	auto windowed_input = this->engine->create_boolean_input();
	auto buttons_input = this->engine->create_boolean_input();

	config_params = {
		{ .key = "playerName", .label = "Player Name", .input = playername_input },
		{ .key = "serverHostname", .label = "Server", .input = serverhostname_input },
		{ .key = "showFPS", .label = "Show FPS", .input = showfps_input },
		{ .key = "showPing", .label = "Show Ping", .input = showping_input },
		#ifndef __ANDROID__
		{ .key = "windowed", .label = "Run windowed", .input = windowed_input },
		#endif
		#ifdef __ANDROID__
		{ .key = "showButtons", .label = "Show buttons", .input = buttons_input },
		#endif
	};

}

void ConfigStage::on_enter_stage() {

	this->welcome_view = !this->engine->get_cfg().json().contains("playerName");

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
		touch_keys.add_button(ALLEGRO_KEY_F1, "F1");
		touch_keys.add_button(ALLEGRO_KEY_ENTER, "Ok");
		touch_keys.add_button(ALLEGRO_KEY_ESCAPE, "Esc");
	} else {
		//touch_keys.add_button(ALLEGRO_KEY_F2, "__kb");
		touch_keys.add_button(ALLEGRO_KEY_ENTER, "Enter");
	}
	touch_keys.fit_buttons(dp::client::ui::FIT_BOTTOM, 10);

	auto& cfg = this->engine->get_cfg().json();
	for (auto& param: config_params) {

		if (!cfg.contains(param.key)) {
			continue;
		}

		param.input->set_from_json_value(cfg.at(param.key));

	} 

}

bool ConfigStage::saved_modified_inputs() {

	for (auto& param: config_params) {

		if (!param.input->is_valid()) {
			return false;
		}
		
	}

	for (auto& param: config_params) {

		// if (modified)
		auto val = param.input->get_json_value();
		this->engine->set_cfg_param(param.key, val);

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

		} else if (keycode == ALLEGRO_KEY_F1) {

			auto& def_cfg = this->engine->get_default_cfg().json();
			auto& curr_param = this->config_params[inp_index];
			const bool has_default = def_cfg.contains(curr_param.key);

			if (has_default) {
				curr_param.input->set_from_json_value(def_cfg.at(curr_param.key));
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

		const string valid_msg = playername_input->get_validation_msg();
		al_draw_text(font, RED, 30, 75, ALLEGRO_ALIGN_LEFT, valid_msg.c_str());

	} else {
		
		auto& cfg = this->engine->get_cfg();

		string player_name = cfg.sget<string>("playerName", "-");
		string msg = "Welcome " + player_name + "!";

		al_draw_text(font, WHITE, 30, 30, ALLEGRO_ALIGN_LEFT, msg.c_str());
		al_draw_text(font, WHITE, 30, 50, ALLEGRO_ALIGN_LEFT, "Press enter to continue");

	}

}


void ConfigStage::draw_config_view() {

	static float frame = 0;

	ALLEGRO_FONT* font = this->engine->get_font();

	auto& def_cfg = this->engine->get_default_cfg().json();
	auto& curr_param = this->config_params[inp_index];
	const bool has_default = def_cfg.contains(curr_param.key);

	if (has_default) {
		al_draw_text(font, al_map_rgb(180, 180, 180), 30, 15, ALLEGRO_ALIGN_LEFT, "Press F1 to revert to default");
	}
	const string valid_msg = curr_param.input->get_validation_msg();
	al_draw_text(font, CGA_BLUE, 30, 30, ALLEGRO_ALIGN_LEFT, valid_msg.c_str());

	const int line_height = 20;

	int y = 50;
	int marker_y = y + inp_index * line_height + 7;

	const float x_shift = 3.0 * sin(frame / 6.0);

	al_draw_filled_circle(16 + x_shift, marker_y, 3, CGA_PINK);
	al_draw_filled_circle(320 - 16 - x_shift, marker_y, 3, CGA_PINK);

	for (auto& param: config_params) {

		al_draw_text(font, WHITE, 30, y, ALLEGRO_ALIGN_LEFT, param.label.c_str());
		if (!param.input->is_valid()) {
			al_draw_text(font, RED, 126, y, ALLEGRO_ALIGN_LEFT, "!");
		}
		param.input->draw(135, y);

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
