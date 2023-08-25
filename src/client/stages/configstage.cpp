#include "configstage.h"
#include "../mediatools.h"
#include <string>

using namespace std;


ConfigStage::ConfigStage(HGameEngine* _engine) : Stage(_engine) {

	this->input = this->engine->create_text_input();

}

void ConfigStage::on_enter_stage() {

	input->focus();

	auto& touch_keys = this->engine->get_touch_keys();

	touch_keys.clear_buttons();
	touch_keys.add_button(ALLEGRO_KEY_ENTER, "Enter");
	touch_keys.add_button(ALLEGRO_KEY_ESCAPE, "Esc");
	touch_keys.fit_buttons(FIT_BOTTOM, 10);

}

void ConfigStage::on_event(ALLEGRO_EVENT event) {

	if (event.type == ALLEGRO_EVENT_KEY_DOWN) {

		int keycode = event.keyboard.keycode;

		if (keycode == ALLEGRO_KEY_ESCAPE) {//ESC (SALIR)


		} else if (keycode == ALLEGRO_KEY_ENTER) {

			if (input->is_focused()) {

				const string name = input->get_value();
				this->engine->set_cfg_param("playerName", name.c_str());
				input->blur();

			} else {

				al_rest(0.2);
				this->engine->set_stage(MENU);

			}

		}

	}

}
/*
void ConfigStage::on_tick() {

	int conn_state = this->engine->get_io_client().get_state();

	if (conn_state == CONNECTION_STATE_CONNECTED_FULL) {

		this->engine->set_stage(LOBBY);
		
	}

}
*/
void ConfigStage::draw() {

	ALLEGRO_FONT* font = this->engine->get_font();
	
	if (input->is_focused()) {

		al_draw_text(font, WHITE, 30, 30, ALLEGRO_ALIGN_LEFT, "Choose a name:");

		input->draw(30, 60);

	} else {
		
		auto& cfg = this->engine->get_cfg();

		string player_name = cfg.contains("playerName") ? cfg["playerName"].as_string().c_str() : "-" ;
		string msg = "Welcome " + player_name + "!";

		al_draw_text(font, WHITE, 30, 30, ALLEGRO_ALIGN_LEFT, msg.c_str());
		al_draw_text(font, WHITE, 30, 50, ALLEGRO_ALIGN_LEFT, "Press enter to continue");

	}

}
