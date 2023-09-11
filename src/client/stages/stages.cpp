#include "stages.h"
#include "onlinestages.h"
#include "../pongclient.h"
#include "../../ponggame.h"
#include <dp/utils.h>
#include <dp/client/mediatools.h>
#include <dp/client/stage.h>

#include <iostream>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <boost/json.hpp>
#include <string>

using dp::client::BaseClient;
using dp::client::ui::TouchKeysCell;
using dp::client::ui::TouchKeysRow;
using dp::client::play_sound;
using dp::client::play_audio;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

typedef struct {
	ALLEGRO_COLOR bar_color;
	string sprite_path;
} bonus_def;

bonus_def bonuses_defs[] = {
	{
		.bar_color = { 0, 0, 0, 0 },
		.sprite_path = RES_DIR"/long.bmp",
	},
	{
		//.bar_color = { 0, 200.0, 50, 0 },
		.bar_color = { 0, 0.78, 0.2, 0 },
		.sprite_path = RES_DIR"/ball.bmp",
	},
	{
		//.bar_color = { 50, 100, 200, 0 },
		.bar_color = { 0.2, 0.39, 0.78, 0 },
		.sprite_path = RES_DIR"/invi.bmp",
	},
	{
		.bar_color = { 0, 0, 0, 0 },
		.sprite_path = RES_DIR"/wall.bmp",
	},
};


void GameHandler::setup(int _play_mode, int _control_mode) {

	this->play_mode = _play_mode;
	this->control_mode = _control_mode;
	
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

void GameHandler::set_player_name(uint8_t player_idx, const string& name) {

	if (player_idx >= 2) {
		return;
	}

	this->players_names[player_idx] = name;

}

string GameHandler::get_player_name(uint8_t player_idx) {

	if (player_idx >= 2) {
		return "";
	}

	string pname = this->players_names[player_idx];
	return pname == "" ? "PLAYER " + std::to_string(player_idx + 1) : pname;

}

/**
 * Retrieves the game CONTROL_* based on control_mode, keycode and player_idx
 */
int GameHandler::get_control(int k_code, uint8_t player_idx) {

	if (this->pong_game->control_mode == CONTROLMODE_TWO_PLAYERS && this->play_mode == PLAYMODE_LOCAL) {

		if (player_idx == 1) {

			if (k_code == ALLEGRO_KEY_UP || k_code == ALLEGRO_KEY_I) return CONTROL_MOVE_UP;
			else if (k_code == ALLEGRO_KEY_DOWN || k_code == ALLEGRO_KEY_K) return CONTROL_MOVE_DOWN;

		} else if (player_idx == 0) {

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

	if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
	
		if (keycode == ALLEGRO_KEY_1) {
			game_handler.setup(PLAYMODE_LOCAL, CONTROLMODE_SINGLE_PLAYER);
			game_handler.set_player_name(0, cfg["playerName"].as_string().c_str());
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
			bool* tuto_completed_ptr = cfg["tutorialCompleted"].if_bool();
			bool tuto_completed = tuto_completed_ptr == nullptr ? false : *tuto_completed_ptr;

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
	al_draw_text(font, WHITE, center, sc * 105, ALLEGRO_ALIGN_CENTER, "Recreated by: Jose Carlos HR");
	al_draw_text(font, WHITE, center, sc * 130, ALLEGRO_ALIGN_CENTER, "1: One Player  2: Two Players");
	al_draw_text(font, WHITE, center, sc * 142, ALLEGRO_ALIGN_CENTER, "3: Training    4: Play online");
	al_draw_text(font, WHITE, center, sc * 154, ALLEGRO_ALIGN_CENTER, "5: Config      ESC: Quit     ");

}


//-----------------------------------------------------------------------------
//------------------------------- [ GameStage ] -------------------------------


GameStage::GameStage(BaseClient* _engine) : Stage(_engine) {

	this->tracer = new Tracer(_engine);

}

void GameStage::draw_court() {

	float scale = this->engine->get_scale();

	float min_court_y = scale * (LIMIT);
	float max_court_y = scale * (MAX_Y - LIMIT);

	al_draw_line(0, min_court_y, scale * DEF_W, min_court_y, WHITE, 2);
	al_draw_line(0, max_court_y, scale * DEF_W, max_court_y, WHITE, 2);
	al_draw_line(scale * (DEF_W / 2 - 1), min_court_y, scale * (DEF_W / 2 - 1), max_court_y, WHITE, 2);
	al_draw_line(scale * (DEF_W / 2 + 1), min_court_y, scale * (DEF_W / 2 + 1), max_court_y, WHITE, 2);

	if (game_handler.pong_game->control_mode == CONTROLMODE_TRAINING) {
		al_draw_line(scale * DEF_W, min_court_y, scale * DEF_W, max_court_y, WHITE, 2);
	}

}


void GameStage::draw_scores() {
	
	ALLEGRO_FONT* font = this->engine->get_font();
	float scale = this->engine->get_scale();

	const float y_coord = 186;

	if (game_handler.pong_game->control_mode == CONTROLMODE_TRAINING) {
		
		al_draw_textf(
			font,
			RED,
			scale * 25,
			scale * y_coord,
			ALLEGRO_ALIGN_LEFT,
			"FAILS:%d",
			game_handler.pong_game->players[1]->score
		);

		return;
	
	}

	/*
	al_draw_textf(
		font,
		RED,
		scale * 25,
		scale * y_coord,
		ALLEGRO_ALIGN_LEFT,
		"SCORE:%d",
		game_handler.pong_game->players[0]->score
	);

	al_draw_textf(
		font,
		RED,
		scale * 240,
		scale * y_coord,
		ALLEGRO_ALIGN_LEFT,
		"SCORE:%d",
		game_handler.pong_game->players[1]->score
	);
	*/

	const float center = DEF_W / 2.0;
	// string player_name = cfg.contains("playerName") ? cfg["playerName"].as_string().c_str() : "-" ;

	al_draw_text(
		font,
		RED,
		scale * (5),
		scale * y_coord,
		ALLEGRO_ALIGN_LEFT,
		game_handler.get_player_name(0).c_str()
	);

	al_draw_text(
		font,
		RED,
		scale * (DEF_W - 5),
		scale * y_coord,
		ALLEGRO_ALIGN_RIGHT,
		game_handler.get_player_name(1).c_str()
	);

	al_draw_textf(
		font,
		RED,
		scale * (center - 12),
		scale * y_coord,
		ALLEGRO_ALIGN_RIGHT,
		"%d",
		game_handler.pong_game->players[0]->score
	);

	for (uint8_t i = 0; i < 4; i++) {

		al_draw_line(
			scale * (center - 1), 
			scale * (DEF_H - 10 + i * 2), 
			scale * (center + 1), 
			scale * (DEF_H - 10 + i * 2), 
			RED,
			0
		);

	}

	al_draw_textf(
		font,
		RED,
		scale * (center + 12),
		scale * y_coord,
		ALLEGRO_ALIGN_LEFT,
		"%d",
		game_handler.pong_game->players[1]->score
	);

}


void GameStage::on_enter_stage() {
	
	auto& touch_keys = this->engine->get_touch_keys();

	touch_keys.clear_buttons();

	if (game_handler.pong_game->control_mode == CONTROLMODE_TWO_PLAYERS && game_handler.play_mode == PLAYMODE_LOCAL) {

		touch_keys.add_button(ALLEGRO_KEY_W, "");
		touch_keys.add_button(ALLEGRO_KEY_UP, "");
		touch_keys.add_button(ALLEGRO_KEY_S, "");
		touch_keys.add_button(ALLEGRO_KEY_DOWN, "");

		TouchKeysCell cell = {
			.width = 1,
			.flex = 1
		};
		TouchKeysRow row = {
			.height = 1,
			.flex = 1,
			.cells = { cell, cell }
		};

		touch_keys.layout_buttons({ row, row });

	} else {

		touch_keys.add_button(ALLEGRO_KEY_DOWN, "");
		touch_keys.add_button(ALLEGRO_KEY_UP, "");

		touch_keys.fit_buttons(dp::client::ui::FIT_HORIZONTAL);

	}


	if (game_handler.play_mode == PLAYMODE_ONLINE) {

		this->engine->get_io_client().set_process_actions_fn([&] (boost::json::object& evt) {

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

		} else if (k_code == ALLEGRO_KEY_D) {

			this->trigger_desync(); //debug

		} else if (k_code == ALLEGRO_KEY_V) {
			
			//game_handler.pong_game->players[1]->give_bonus(BONUS_INVI);//debug
			//game_handler.pong_game->players[0]->give_bonus(BONUS_WALL);//debug

		}

	}

	if (evt.type == ALLEGRO_EVENT_KEY_DOWN || evt.type == ALLEGRO_EVENT_KEY_UP) {

		bool new_st = evt.type == ALLEGRO_EVENT_KEY_DOWN;

		int control_p1 = game_handler.get_control(k_code, 0);
		int control_p2 = game_handler.get_control(k_code, 1);

		if (game_handler.play_mode == PLAYMODE_ONLINE) {
				
			if (control_p1 != CONTROL_NONE) {

				boost::json::value input_evt = {
					{"type", "set_control_state"},
					{"state", new_st},
					{"control", control_p1},
					{"tick", game_handler.pong_game->tick}
				};

				cout << "Sending: " << input_evt << endl;
				this->engine->get_io_client().qsend_udp(boost::json::serialize(input_evt));

			}

		} else {
		
			game_handler.pong_game->players[0]->controls[control_p1] = new_st;
			game_handler.pong_game->players[1]->controls[control_p2] = new_st;
			
		} 

	}
	
}


void GameStage::process_message(string &msg) {

	if (msg == "scored") {

		play_sound(Re, 130);
		play_sound(Do, 250);
		play_audio();

	} else if (msg == "hit") {

		play_sound(Mi, 40, 4);
		play_audio();

	}

}


void GameStage::trigger_desync() {
	
	boost::json::object pkg = {{"type", "desync"}}; //play_again
	
	this->engine->get_io_client().qsend(boost::json::serialize(pkg));

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

			cerr << "Error during game: " << e.what() << endl;
			
			this->trigger_desync();

		}

	} else {

		if (this->engine->get_key(ALLEGRO_KEY_G)) {
			//game_handler.pong_game->players[0]->medlen += 1;//DEBUG
		}

	}

	game_handler.pong_game->process_tick();

	while (!game_handler.pong_game->messages.empty()) {
		string msg = game_handler.pong_game->messages.front();
		game_handler.pong_game->messages.pop();
		this->process_message(msg);
	}

	if (game_handler.pong_game->finished) {

		const uint8_t local_player_idx = game_handler.play_mode == PLAYMODE_ONLINE ? game_handler.local_player_idx : 0;
		
		this->engine->set_stage(OVER);

		const bool local_player_wins = local_player_idx == game_handler.pong_game->get_winner_idx();

		if (local_player_wins) {
			play_sound(Do, 150, 3);
			play_sound(Mi, 150, 3);
			play_sound(Sol, 200, 3);
			play_sound(Do, 700, 4);
		} else {
			play_sound(Re, 150, 3);
			play_sound(Re, 150, 3);
			play_sound(Re, 200, 3);
			play_sound(LaSos, 500, 2);
		}
		
		play_audio();
		
	}
	
}


void GameStage::draw() {

	ALLEGRO_FONT* font = this->engine->get_font();

	float scale = this->engine->get_scale();
	
	
	if (delayer > 0) {

		al_draw_textf(
			font, 
			RED, 
			scale * this->engine->get_res_x() / 2, 
			scale * this->engine->get_res_y() / 2, 
			ALLEGRO_ALIGN_CENTER, 
			"%d", 
			1 + delayer / 25
		);
		
	} else {

		if (game_handler.pong_game->paused) {

			al_draw_text(
				font, 
				al_map_rgb(0, 200, 100), 
				this->engine->get_res_x() * scale / 2, 
				this->engine->get_res_y() * scale / 2 - 5, 
				ALLEGRO_ALIGN_CENTER, 
				"PAUSA"
			);
			
		} else {

			//al_draw_text(font, RED, scale * DEF_W / 2, scale * 2, ALLEGRO_ALIGN_CENTER, "Press ESC to Main Menu");
			
			int secs = game_handler.pong_game->tick / 60;
			int secs_d = (secs % 60) / 10;
			int secs_u = secs % 10;
			int min = secs / 60;
			al_draw_textf(font, RED, scale * DEF_W / 2, 0, ALLEGRO_ALIGN_CENTER, "%d:%d%d", min, secs_d, secs_u);
				
			this->draw_court();

			Tracer *tr = this->tracer;

			for (auto& w: game_handler.pong_game->walls) {
				tr->draw_wall(w, scale);
			}

			tr->draw_ball(game_handler.pong_game->ball, scale);
			
			for (uint8_t i = 0; i < BONUS_MAX; i++) {
				tr->draw_bonus(game_handler.pong_game->bonus[i], scale);
			}

			this->draw_scores();

			//al_draw_textf(font, RED, scale * 160, scale * 186, ALLEGRO_ALIGN_CENTER, "FPS: %d", (int) (this->engine->get_fps()));
			al_draw_textf(font, RED, scale * 5, 0, ALLEGRO_ALIGN_LEFT, "FPS: %d", (int) (this->engine->get_fps()));

			if (game_handler.play_mode == PLAYMODE_ONLINE) {
				int ping = (int) (this->engine->get_io_client().get_ping_ms());
				al_draw_textf(font, RED, scale * (DEF_W - 5), scale * 1, ALLEGRO_ALIGN_RIGHT, "PING: %d", ping);
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


Tracer::Tracer(BaseClient* _engine) :
	engine(_engine)
{

	for (uint8_t i = 0; i < BONUS_MAX; i++) {
		// TODO: quiza mejor seria engine->load_bitmap_resource
		this->bonus_sprites[i] = dp::client::load_bitmap(bonuses_defs[i].sprite_path);
	}

}

ALLEGRO_BITMAP* Tracer::get_sprite_for_bonus_type(BonusType bonus_type) {

	if (bonus_type >= 0 && bonus_type < BONUS_MAX) {

		return this->bonus_sprites[bonus_type];

	} else {

		cerr << "Unknown bonus type: " << bonus_type << endl;
		return nullptr;

	}

}

void Tracer::draw_ball(Ball *b, float scale) {
		
	if (!b->stat) {
		return;
	}

	uint8_t invisiball_state = b->get_invisiball_state();

	if (invisiball_state == 0) {
		al_draw_filled_circle(scale * b->x, scale * b->y, scale * b->radius, WHITE);
	} else if (invisiball_state == 1) {
		al_draw_circle(scale * b->x, scale * b->y, scale * b->radius, WHITE, 0);
	}

}

void Tracer::draw_bonus(Bonus *b, float scale) {
	
	if (!b->stat) {
		return;
	}

	ALLEGRO_BITMAP *spr = this->get_sprite_for_bonus_type(b->bonus_type);
	if (spr != nullptr) {
		al_draw_bitmap(spr, scale * (b->x - 10), scale * (b->y - 10), 0);
		// x-sprite->w/2, y-sprite->h/2);
	}

}

void Tracer::draw_wall(Wall* w, float scale) {
	
	if (!w->stat) {
		return;
	}

	const float x = w->get_x();
	const float y0 = w->get_y() - w->radius;
	const float y1 = w->get_y() + w->radius;

	const float dir = w->owner_idx == 0 ? 1.0 : -1.0;

	const bool active = (w->owner_idx == 0 && w->game->ball->get_vx() < 0) || (w->owner_idx == 1 && w->game->ball->get_vx() > 0);

	const ALLEGRO_COLOR color = active ? WHITE : al_map_rgb(80, 90, 80);

	al_draw_line(x + 1 * dir, y0 - 2, x, y0, color, 2.0);
	al_draw_line(x, y0, x, y1, color, 2.0);
	al_draw_line(x, y1, x + 1 * dir, y1 + 2, color, 2.0);

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
	
	int y_offset = 0;
	const uint8_t line_width = 4;

	for (uint8_t i = 0; i < BONUS_MAX; i++) {

		if (!pl->bonus_timers[i]) {
			continue;
		}

		if (pl->x < 100) {

			al_draw_filled_rectangle(
				scale * 0,
				scale * (16 + y_offset),
				scale * (158.0 * pl->bonus_timers[i] / 800.0),
				scale * (16 + y_offset + line_width),
				bonuses_defs[i].bar_color
			);

		} else {

			al_draw_filled_rectangle(
				scale * (DEF_W),
				scale * (16 + y_offset),
				scale * (DEF_W - 158.0 * pl->bonus_timers[i] / 800.0),
				scale * (16 + y_offset + line_width),
				bonuses_defs[i].bar_color
			);

		}

		y_offset += line_width;
	
	}

	if (pl->com_txt_y >- 40) {

		int txt_x;

		if (pl->x < 100) txt_x = 60;
		else txt_x = 220;

		al_draw_text(
			this->engine->get_font(), 
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

	auto& touch_keys = this->engine->get_touch_keys();

	touch_keys.clear_buttons();

	touch_keys.add_button(ALLEGRO_KEY_Y, "Y");
	touch_keys.add_button(ALLEGRO_KEY_N, "N");

	touch_keys.fit_buttons(dp::client::ui::FIT_BOTTOM, 10);
	
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

	int winner_id = game_handler.pong_game->get_winner_idx();
	const string winner = game_handler.get_player_name(winner_id);

	float scale = this->engine->get_scale();
	ALLEGRO_FONT *font = this->engine->get_font();

	al_draw_textf(font, RED, scale * DEF_W / 2, scale * 54, ALLEGRO_ALIGN_CENTER, "WINNER: %s", winner.c_str());
	al_draw_text (font, RED, scale * DEF_W / 2, scale * 70, ALLEGRO_ALIGN_CENTER, "PLAY AGAIN? (Y/N)");

}
