#include "../ponggame.h"
#include "../utils.h"
#include "mediatools.h"
#include "hengine.h"
#include "stages.h"
#include "onlinestages.h"

#include <iostream>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <boost/json.hpp>
#include <string>
#include <cmath>

using namespace std;


void GameHandler::setup(int _playMode, int _controlMode) {

	this->play_mode = _playMode;
	this->control_mode = _controlMode;
	
}

void GameHandler::make_new_pong_game(int_fast32_t seed) {

	this->cleanup();

	pong_game = new PongGame(seed);

	pong_game->control_mode = this->control_mode;

}

void GameHandler::cleanup() {

	delete pong_game;

}

GameHandler::~GameHandler() {

	this->cleanup();

}

/**
 * Retrieves the game CONTROL_* based on control_mode, keycode and player_id
 */
int GameHandler::get_control(int k_code, int player_id) {

	if (this->pong_game->control_mode == CONTROLMODE_TWO_PLAYERS && this->play_mode == PLAYMODE_LOCAL) {

		if (player_id == 1) {

			if (k_code == ALLEGRO_KEY_UP || k_code == ALLEGRO_KEY_I) return CONTROL_MOVE_UP;
			else if (k_code == ALLEGRO_KEY_DOWN || k_code == ALLEGRO_KEY_K) return CONTROL_MOVE_DOWN;

		} else if (player_id == 0) {

			if (k_code == ALLEGRO_KEY_W) return CONTROL_MOVE_UP;
			else if (k_code == ALLEGRO_KEY_S) return CONTROL_MOVE_DOWN;

		}

	} else {

		if (k_code == ALLEGRO_KEY_UP) return CONTROL_MOVE_UP;
		else if (k_code == ALLEGRO_KEY_DOWN) return CONTROL_MOVE_DOWN;

	}

	return CONTROL_NONE;

}



GameHandler game_handler;



//-----------------------------------------------------------------------------
//------------------------------ MainMenuStage --------------------------------

MainMenuStage::MainMenuStage(HGameEngine* _engine):Stage(_engine) {
	
	this->logo = load_bitmap(LOGO_DIR);

	//std::cout << "MainMenuStage init" << std::endl;

}


void MainMenuStage::on_enter_stage() {

	this->engine->touch_keys.clear_buttons();

	this->engine->touch_keys.add_button(ALLEGRO_KEY_1, "1");
	this->engine->touch_keys.add_button(ALLEGRO_KEY_2, "2");
	this->engine->touch_keys.add_button(ALLEGRO_KEY_3, "3");
	this->engine->touch_keys.add_button(ALLEGRO_KEY_4, "4");
	this->engine->touch_keys.add_button(ALLEGRO_KEY_ESCAPE, "ESC");

	this->engine->touch_keys.fit_buttons(FIT_BOTTOM, 10);
}


void MainMenuStage::on_tick() {

	if (easteregg++ == 5000) play_exorcista();

}
	

void MainMenuStage::on_event(ALLEGRO_EVENT event) {

	int keycode = event.keyboard.keycode;

	if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
	
		if (keycode == ALLEGRO_KEY_1) {
			game_handler.setup(PLAYMODE_LOCAL, CONTROLMODE_SINGLE_PLAYER);
		} 
		else if (keycode == ALLEGRO_KEY_2) {
			game_handler.setup(PLAYMODE_LOCAL, CONTROLMODE_TWO_PLAYERS);
		}
		else if (keycode == ALLEGRO_KEY_3) {
			game_handler.setup(PLAYMODE_LOCAL, CONTROLMODE_TRAINING);
		}
		else if (keycode == ALLEGRO_KEY_4) {
			this->engine->set_stage(CONN); 
			game_handler.setup(PLAYMODE_ONLINE, CONTROLMODE_TWO_PLAYERS);
		}
		else if (keycode == ALLEGRO_KEY_5) {
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

		else if (keycode == ALLEGRO_KEY_1 || keycode == ALLEGRO_KEY_2 || keycode == ALLEGRO_KEY_3 || keycode == ALLEGRO_KEY_5) {

			game_handler.make_new_pong_game(time(nullptr)); 
			this->engine->set_stage(GAME);

		}
	}

}


void MainMenuStage::draw() {

	float sc = this->engine->scale;
	ALLEGRO_FONT* font = this->engine->font;

	al_draw_bitmap(this->logo, (sc - 1) * DEF_W / 2, (sc - 1) * 50, 0);
	al_draw_text(font, al_map_rgb(255, 255, 255), sc * DEF_W / 2, sc * 105, ALLEGRO_ALIGN_CENTER, "Recreated by: Jose Carlos HR");
	al_draw_text(font, al_map_rgb(255, 255, 255), sc * DEF_W / 2, sc * 130, ALLEGRO_ALIGN_CENTER, "1:One Player  2:Two Players");
	al_draw_text(font, al_map_rgb(255, 255, 255), sc * DEF_W / 2, sc * 140, ALLEGRO_ALIGN_CENTER, "3:Training    4:Play online");
	al_draw_text(font, al_map_rgb(255, 255, 255), sc * DEF_W / 2, sc * 155, ALLEGRO_ALIGN_CENTER, "ESC: Quit");

}


//-----------------------------------------------------------------------------
//------------------------------- [ GameStage ] -------------------------------


GameStage::GameStage(HGameEngine* _engine):Stage(_engine) {
   
	this->tracer = new Tracer(_engine);

}

void GameStage::drawCourt() {

	float scale = this->engine->scale;

	float min_court_y = scale * (LIMIT);
	float max_court_y = scale * (MAX_Y - LIMIT);

	al_draw_line(0, min_court_y, scale * DEF_W, min_court_y, al_map_rgb(255, 255, 255), 2);
	al_draw_line(0, max_court_y, scale * DEF_W, max_court_y, al_map_rgb(255, 255, 255), 2);
	al_draw_line(scale * (320 / 2 - 1), min_court_y, scale * (320 / 2 - 1), max_court_y, al_map_rgb(255, 255, 255), 2);
	al_draw_line(scale * (320 / 2 + 1), min_court_y, scale * (320 / 2 + 1), max_court_y, al_map_rgb(255, 255, 255), 2);

	if (game_handler.pong_game->control_mode == CONTROLMODE_TRAINING) {
		al_draw_line(scale * DEF_W, min_court_y, scale * DEF_W, max_court_y, al_map_rgb(255, 255, 255), 2);
	}

}


void GameStage::drawScores() {
	
	ALLEGRO_FONT* font = this->engine->font;
	float scale = this->engine->scale;

	if (game_handler.pong_game->control_mode != CONTROLMODE_TRAINING) {

		al_draw_textf(
			font,
			al_map_rgb(255, 0, 0),
			scale * 25,
			scale * 186,
			ALLEGRO_ALIGN_LEFT,
			"SCORE:%d",
			game_handler.pong_game->players[0]->score
		);

		al_draw_textf(
			font,
			al_map_rgb(255, 0, 0),
			scale * 240,
			scale * 186,
			ALLEGRO_ALIGN_LEFT,
			"SCORE:%d",
			game_handler.pong_game->players[1]->score
		);
	
	} else {

		al_draw_textf(
			font,
			al_map_rgb(255, 0, 0),
			scale * 25,
			scale * 186,
			ALLEGRO_ALIGN_LEFT,
			"FAILS:%d",
			game_handler.pong_game->players[1]->score
		);
	
	}

}


void GameStage::on_enter_stage() {
	
	this->engine->touch_keys.clear_buttons();

	this->engine->touch_keys.add_button(ALLEGRO_KEY_DOWN, "");
	this->engine->touch_keys.add_button(ALLEGRO_KEY_UP, "");

	this->engine->touch_keys.fit_buttons(FIT_HORIZONTAL);

	if (game_handler.play_mode == PLAYMODE_ONLINE) {

		this->engine->connection.set_process_actions_fn([&] (boost::json::object& evt) {

			controller.push_event(evt);
			
		});

	}

	play_sound(Do, 400);
	play_sound(Re, 200);
	play_sound(La, 100);
	play_sound(Si, 100);
	play_audio();

	game_handler.pong_game->restart();
	game_handler.pong_game->iniciar_punto(1);

	delayer = 75;

}


void GameStage::on_event(ALLEGRO_EVENT evt) {

	int k_code = evt.keyboard.keycode;
	
	if (evt.type == ALLEGRO_EVENT_KEY_DOWN) {

		if (k_code == ALLEGRO_KEY_ESCAPE) {

			this->engine->set_stage(MENU);

		} else if (k_code == ALLEGRO_KEY_P) {//P (PAUSA)

			if (game_handler.play_mode == PLAYMODE_ONLINE) {

			} else {

				game_handler.pong_game->toggle_pause();

			}

		}

	}

	if (evt.type == ALLEGRO_EVENT_KEY_DOWN || evt.type == ALLEGRO_EVENT_KEY_UP) {

		bool newSt = evt.type == ALLEGRO_EVENT_KEY_DOWN;

		int controlP1 = game_handler.get_control(k_code, 0);
		int controlP2 = game_handler.get_control(k_code, 1);

		if (game_handler.play_mode == PLAYMODE_ONLINE) {
				
			if (controlP1 != CONTROL_NONE) {

				boost::json::value inputEvt;
				//inputEvt.emplace_null();
			
				inputEvt = {
					{"type", "set_control_state"},
					{"state", newSt},
					{"control", controlP1},
					{"tick", game_handler.pong_game->tick}
				};

				cout << "Sending: " << inputEvt << endl;
				this->engine->connection.qsend(boost::json::serialize(inputEvt));//send inputEvt

			}

		} else {
		
			game_handler.pong_game->players[0]->controls[controlP1] = newSt;

			game_handler.pong_game->players[1]->controls[controlP2] = newSt;
			
		} 

	}
	
}


void GameStage::processMessage(string &msg) {

	if (msg == "scored") {

		play_sound(Re, 130);
		play_sound(Do, 250);
		play_audio();

	} else if (msg == "hit") {

		play_sound(Mi, 40, 4);
		play_audio();

	}

}


void GameStage::on_tick() {

	if (delayer > 0) {
		delayer--;
		return;
	} 

	if (game_handler.pong_game->paused) {
		return;
	}

	if (game_handler.play_mode == PLAYMODE_ONLINE) {

		try {

			controller.on_tick();

		} catch (std::exception& e) {

			boost::json::object pkg = {{"type", "desync"}}; //play_again
			
			this->engine->connection.qsend(boost::json::serialize(pkg));

		}

	} else {

		if (this->engine->keys[ALLEGRO_KEY_G]) {
			//game_handler.pong_game->players[0]->medlen += 1;//DEBUG
		}

	}

	game_handler.pong_game->process_tick();

	while (!game_handler.pong_game->messages.empty()) {
		string msg = game_handler.pong_game->messages.front();
		game_handler.pong_game->messages.pop();
		this->processMessage(msg);
	}

	if (game_handler.pong_game->finished) {
		
		this->engine->set_stage(OVER);
		play_sound(Re, 150, 3);
		play_sound(Re, 150, 3);
		play_sound(Re, 200, 3);
		play_sound(LaSos, 500, 2);
		play_audio();

	}
	
}


void GameStage::draw() {

	ALLEGRO_FONT* font = this->engine->font;

	float scale = this->engine->scale;
	
	if (delayer > 0) {

		al_draw_textf(
			font, 
			al_map_rgb(255, 0, 0), 
			scale * this->engine->res_x / 2, 
			scale * this->engine->res_y / 2, 
			ALLEGRO_ALIGN_CENTER, 
			"%d", 
			1 + delayer / 25
		);
		
	} else {

		if (game_handler.pong_game->paused) {

			al_draw_text(
				font, 
				al_map_rgb(0, 200, 100), 
				this->engine->res_x * scale / 2, 
				this->engine->res_y * scale / 2 - 5, 
				ALLEGRO_ALIGN_CENTER, 
				"PAUSA"
			);
			
		} else {

			//al_draw_text(font, al_map_rgb(255, 0, 0), scale * 320 / 2, scale * 2, ALLEGRO_ALIGN_CENTER, "Press ESC to Main Menu");
			
			int secs = game_handler.pong_game->tick / 60;
			int secs_d = (secs % 60) / 10;
			int secs_u = secs % 10;
			int min = secs / 60;
			al_draw_textf(font, al_map_rgb(255, 0, 0), scale * 320 / 2, scale * 1, ALLEGRO_ALIGN_CENTER, "%d:%d%d", min, secs_d, secs_u);
				
			this->drawCourt();

			Tracer *tr = this->tracer;

			tr->draw_ball(game_handler.pong_game->ball, scale);
			tr->draw_bonus(game_handler.pong_game->bonus[0], scale);
			tr->draw_bonus(game_handler.pong_game->bonus[1], scale);

			this->drawScores();

			al_draw_textf(font, al_map_rgb(255, 0, 0), scale * 160, scale * 186, ALLEGRO_ALIGN_CENTER, "FPS: %d", (int) (this->engine->fps));

			if (game_handler.play_mode == PLAYMODE_ONLINE) {
				al_draw_textf(font, al_map_rgb(255, 0, 0), scale * (320 - 4), scale * 1, ALLEGRO_ALIGN_RIGHT, "PING: %d", (int) (this->engine->connection.ping_ms));
			}

			tr->draw_player(game_handler.pong_game->players[0], scale);
			if (game_handler.pong_game->control_mode != CONTROLMODE_TRAINING) {
				tr->draw_player(game_handler.pong_game->players[1], scale);
			}

		}

	}

}



//-----------------------------------------------------------------------------
//---------------------------------[ TRACER ]----------------------------------


Tracer::Tracer(HGameEngine* _engine) {

	this->engine = _engine;

	this->bonus_ball_spr = load_bitmap(BALL_DIR);

	this->bonus_long_spr = load_bitmap(LONG_DIR);

}

ALLEGRO_BITMAP* Tracer::get_sprite_for_bonus_type(int bonus_type) {

	if (bonus_type == BONUS_BALL) {

		return this->bonus_ball_spr;

	} else if (bonus_type == BONUS_LONG) {

		return this->bonus_long_spr;

	} else {

		cerr << "Unknown bonus type: " << bonus_type << endl;

		return nullptr;

	}

}

void Tracer::draw_ball(Ball *b, float scale) {
		
	if (!b->stat) {
		return;
	}

	al_draw_filled_circle(scale * b->x, scale * b->y, scale * b->radius, al_map_rgb( 255,255, 255));

}

void Tracer::draw_bonus(Bonus * b, float scale) {
	
	if (!b->stat) {
		return;
	}

	ALLEGRO_BITMAP *spr = this->get_sprite_for_bonus_type(b->bonus_type);
	if (spr != nullptr) {
		al_draw_bitmap(spr, scale * (b->x - 10), scale * (b->y - 10), 0);
		// x-sprite->w/2, y-sprite->h/2);
	}

}


void Tracer::draw_player(PlayerP *pl, int scale) {

	int medln = pl->medlen;
	
	al_draw_filled_rectangle(
		scale * (pl->x - 2), 
		scale * (pl->y - medln), 
		scale * (pl->x + 2), 
		scale * (pl->y + medln), 
		WHITE
	);

	al_draw_filled_rectangle(
		scale * (pl->x - 1),
		scale * (pl->y - medln - 1), 
		scale * (pl->x + 1), 
		scale * (pl->y + medln + 1), 
		WHITE
	);
	
	if (pl->bonus_timers[BONUS_BALL]) {

		if (pl->x < 100) {

			al_draw_filled_rectangle(
				scale * 0, 
				scale * 16, 
				scale * (158.0 * pl->bonus_timers[BONUS_BALL] / 800.0), 
				scale * 20, 
				al_map_rgb(0, 200, 50)
			);

		} else {

			al_draw_filled_rectangle(
				scale * (DEF_W), 
				scale * 16, 
				scale * (DEF_W - 158.0 * pl->bonus_timers[BONUS_BALL] / 800.0),
				scale * 20, 
				al_map_rgb(0, 200, 50)
			);

		}
	
	}

	if (pl->com_txt_y >- 40) {

		int txt_x;

		if (pl->x < 100) txt_x = 60;
		else txt_x = 220;

		al_draw_text(
			this->engine->font, 
			al_map_rgb(150 - pl->com_txt_y, 150 - pl->com_txt_y * 4, 255), 
			scale * txt_x, 
			scale * pl->com_txt_y, 
			ALLEGRO_ALIGN_CENTER, 
			pl->com_txt.c_str()
		);

		pl->com_txt_y -= 2;
	}

}



//-----------------------------------------------------------------------------
//------------------------------- GameOverStage -------------------------------


void GameOverStage::on_enter_stage() {

	this->engine->touch_keys.clear_buttons();

	this->engine->touch_keys.add_button(ALLEGRO_KEY_Y, "Y");
	this->engine->touch_keys.add_button(ALLEGRO_KEY_N, "N");

	this->engine->touch_keys.fit_buttons(FIT_BOTTOM, 10);
	
}

void GameOverStage::on_event(ALLEGRO_EVENT event) {

	if (event.type == ALLEGRO_EVENT_KEY_DOWN) {

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

	int winner;
	if (game_handler.pong_game->players[0]->score > game_handler.pong_game->players[1]->score) winner = 1;
	else winner = 2;

	float scale = this->engine->scale;
	ALLEGRO_FONT *font = this->engine->font;

	al_draw_textf(font, al_map_rgb(255, 0, 0), scale * 320 / 2, scale * 54, ALLEGRO_ALIGN_CENTER, "WINNER: PLAYER %d", winner);
	al_draw_text (font, al_map_rgb(255, 0, 0), scale * 320 / 2, scale * 70, ALLEGRO_ALIGN_CENTER, "PLAY AGAIN? (Y/N)");

}
