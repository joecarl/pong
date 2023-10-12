#include "stages.h"
#include "../pongclient.h"
#include <dp/client/stage.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <boost/json.hpp>
#include <iostream>
#include <string>

using dp::client::BaseClient;
using std::string;
using std::cout;
using std::cerr;
using std::endl;


//-----------------------------------------------------------------------------
//------------------------------ MainMenuStage --------------------------------

MainMenuStage::MainMenuStage(BaseClient* _engine) :
	Stage(_engine),
	retro_logo({
		"------    -----    -----    -----  ",
		"--   --  --   --  --   --  --   -- ",
		"--   --  --   --  --   --  --      ",
		"------   --   --  --   --  --  --- ",
		"--       --   --  --   --  --   -- ",
		"--        -----   --   --   -----  "
	})
{
	
	//this->logo = load_bitmap(LOGO_DIR);

	//std::cout << "MainMenuStage init" << std::endl;

}


void MainMenuStage::on_enter_stage() {

	auto& touch_keys = this->engine->get_touch_keys();

	touch_keys.clear_buttons();

	touch_keys.add_button(ALLEGRO_KEY_1, "1");
	touch_keys.add_button(ALLEGRO_KEY_2, "2");
	touch_keys.add_button(ALLEGRO_KEY_3, "3");
	touch_keys.add_button(ALLEGRO_KEY_4, "4");
	touch_keys.add_button(ALLEGRO_KEY_5, "5");
	touch_keys.add_button(ALLEGRO_KEY_ESCAPE, "ESC");

	touch_keys.fit_buttons(dp::client::ui::FIT_BOTTOM, 10);

	//play_main_theme();

}


void MainMenuStage::on_tick() {

	//if (easteregg++ == 5000) play_exorcista();

}
	

void MainMenuStage::on_event(ALLEGRO_EVENT event) {

	int keycode = event.keyboard.keycode;
	auto& cfg = this->engine->get_cfg();

	PongClient* cl = static_cast<PongClient*>(this->engine);
	auto& game_handler = cl->get_game_handler();

	if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
	
		if (keycode == ALLEGRO_KEY_1) {
			game_handler.setup(PLAYMODE_LOCAL, CONTROLMODE_SINGLE_PLAYER);
			game_handler.set_player_name(0, cfg["playerName"]);
			game_handler.set_player_name(1, "PC");
		} 
		else if (keycode == ALLEGRO_KEY_2) {
			game_handler.setup(PLAYMODE_LOCAL, CONTROLMODE_TWO_PLAYERS);
			game_handler.set_player_name(0, "PLAYER 1");
			game_handler.set_player_name(1, "PLAYER 2");
		}
		else if (keycode == ALLEGRO_KEY_3) {
			game_handler.setup(PLAYMODE_LOCAL, CONTROLMODE_TRAINING);
		}
		else if (keycode == ALLEGRO_KEY_4) {
			this->engine->set_stage(CONN); 
			game_handler.setup(PLAYMODE_ONLINE, CONTROLMODE_TWO_PLAYERS);
		}
		else if (keycode == ALLEGRO_KEY_5) {
			this->engine->set_stage(CONF); 
		}
		else if (keycode == ALLEGRO_KEY_9) {
			game_handler.setup(PLAYMODE_LOCAL, CONTROLMODE_DEBUG);
		}
		
		if (keycode == ALLEGRO_KEY_C) {//CAMBIAR RESOLUCION, bad performance
			/*
			scale = scale == 1 ? 2 : 1;
			al_destroy_font(font);
			al_destroy_bitmap(buffer);
			font = al_load_ttf_font("resources/font.ttf", scale * 9, 0) ;
			buffer = al_create_bitmap(scale * res_x, scale * res_y);
			*/
		}

		if (keycode == ALLEGRO_KEY_ESCAPE) {
			this->engine->finish = true;
		}

		else if (keycode == ALLEGRO_KEY_1 || keycode == ALLEGRO_KEY_2 || keycode == ALLEGRO_KEY_3 || keycode == ALLEGRO_KEY_9) {

			game_handler.make_new_pong_game(time(nullptr));
			bool tuto_completed = cfg.sget<bool>("tutorialCompleted");

			if (keycode == ALLEGRO_KEY_1 && !tuto_completed) {
				this->engine->set_stage(TUTO);
			} else {
				this->engine->set_stage(GAME);
			}

		}
	}

}


void MainMenuStage::draw() {

	float sc = this->engine->get_scale();
	ALLEGRO_FONT* font = this->engine->get_font();

	retro_logo.draw((sc * DEF_W - retro_logo.get_width()) / 2, sc * 40);
	//al_draw_bitmap(this->logo, (sc - 1) * DEF_W / 2, (sc - 1) * 50, 0);
	const float center = sc * DEF_W / 2;
	al_draw_text(font, al_map_rgb(140, 140, 140), center, sc * 105, ALLEGRO_ALIGN_CENTER, "Recreated by: Jose Carlos HR");
	al_draw_text(font, WHITE, center, sc * 130, ALLEGRO_ALIGN_CENTER, "1: One Player  2: Two Players");
	al_draw_text(font, WHITE, center, sc * 142, ALLEGRO_ALIGN_CENTER, "3: Training    4: Play online");
	al_draw_text(font, WHITE, center, sc * 154, ALLEGRO_ALIGN_CENTER, "5: Config      ESC: Quit     ");

}



//-----------------------------------------------------------------------------
//------------------------------- GameOverStage -------------------------------


void GameOverStage::on_enter_stage() {

	auto& touch_keys = this->engine->get_touch_keys();

	touch_keys.clear_buttons();

	touch_keys.add_button(ALLEGRO_KEY_Y, "Y");
	touch_keys.add_button(ALLEGRO_KEY_N, "N");

	touch_keys.fit_buttons(dp::client::ui::FIT_BOTTOM, 10);
	
}

void GameOverStage::on_event(ALLEGRO_EVENT event) {

	if (event.type == ALLEGRO_EVENT_KEY_DOWN) {

		PongClient* cl = static_cast<PongClient*>(this->engine);
		auto& game_handler = cl->get_game_handler();
		int keycode = event.keyboard.keycode;

		if (keycode == ALLEGRO_KEY_Y) {
			if (game_handler.play_mode == PLAYMODE_ONLINE) {

				this->engine->set_stage(LOBBY);

			} else {
				
				this->engine->set_stage(GAME);

			}
		} else if (keycode == ALLEGRO_KEY_N) {
			this->engine->set_stage(MENU);
		}

	}

}

void GameOverStage::draw() {

	PongClient* cl = static_cast<PongClient*>(this->engine);
	auto& game_handler = cl->get_game_handler();

	int winner_id = game_handler.pong_game->get_winner_idx();
	const string winner = game_handler.get_player_name(winner_id);

	float scale = this->engine->get_scale();
	ALLEGRO_FONT *font = this->engine->get_font();

	al_draw_textf(font, CGA_PINK, scale * DEF_W / 2, scale * 54, ALLEGRO_ALIGN_CENTER, "WINNER: %s", winner.c_str());
	al_draw_text (font, CGA_PINK, scale * DEF_W / 2, scale * 70, ALLEGRO_ALIGN_CENTER, "PLAY AGAIN? (Y/N)");

}
