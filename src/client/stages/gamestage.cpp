#include "gamestage.h"
#include "../pongclient.h"
#include <dp/client/stage.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <iostream>
#include <string>

using dp::client::BaseClient;
using dp::client::ui::TouchKeysCell;
using dp::client::ui::TouchKeysRow;
using dp::client::ui::Button;
using std::string;
using std::cout, std::cerr, std::endl;

typedef struct {
	ALLEGRO_COLOR bar_color;
	string sprite_path;
} BonusDef;

BonusDef bonuses_defs[] = {
	{
		.bar_color = { 0, 0, 0, 0 },
		.sprite_path = "long.bmp",
	},
	{
		//.bar_color = { 0, 200.0, 50, 0 },
		.bar_color = { 0, 0.78, 0.2, 0 },
		.sprite_path = "ball.bmp",
	},
	{
		//.bar_color = { 50, 100, 200, 0 },
		.bar_color = { 0.2, 0.39, 0.78, 0 },
		.sprite_path = "invi.bmp",
	},
	{
		.bar_color = { 0, 0, 0, 0 },
		.sprite_path = "wall.bmp",
	},
};

//-----------------------------------------------------------------------------
//------------------------------- [ GameStage ] -------------------------------


GameStage::GameStage(BaseClient* _engine) : Stage(_engine) {

	this->tracer = new Tracer(_engine);

}

void GameStage::draw_court() {
	
	PongClient* cl = static_cast<PongClient*>(this->engine);
	auto& game_handler = cl->get_game_handler();
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
	
	PongClient* cl = static_cast<PongClient*>(this->engine);
	auto& game_handler = cl->get_game_handler();
	ALLEGRO_FONT* font = this->engine->get_font();
	float scale = this->engine->get_scale();

	const float y_coord = 186;

	if (game_handler.pong_game->control_mode == CONTROLMODE_TRAINING) {
		
		al_draw_textf(
			font,
			CGA_BLUE,
			scale * 25,
			scale * y_coord,
			ALLEGRO_ALIGN_LEFT,
			"FAILS:%d",
			game_handler.pong_game->players[1]->score
		);

		return;
	
	}

	const float center = DEF_W / 2.0;

	al_draw_text(
		font,
		CGA_PINK,
		scale * (5),
		scale * y_coord,
		ALLEGRO_ALIGN_LEFT,
		game_handler.get_player_name(0).c_str()
	);

	al_draw_text(
		font,
		CGA_PINK,
		scale * (DEF_W - 5),
		scale * y_coord,
		ALLEGRO_ALIGN_RIGHT,
		game_handler.get_player_name(1).c_str()
	);

	al_draw_textf(
		font,
		CGA_PINK,
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
			CGA_BLUE,
			0
		);

	}

	al_draw_textf(
		font,
		CGA_PINK,
		scale * (center + 12),
		scale * y_coord,
		ALLEGRO_ALIGN_LEFT,
		"%d",
		game_handler.pong_game->players[1]->score
	);

}


void draw_up_shape(float x, float y, float size, ALLEGRO_COLOR c) {

	float r = size / 2.0;
	const float vtx[] = {				
		x + r, y + r,
		x + r, y - (r / 3.0f),
		x + (r / 3.0f), y - r,
		x - (r / 3.0f), y - r,
		x - r, y - (r / 3.0f),
		x - r, y + r,
	};
	al_draw_filled_polygon(vtx, sizeof(vtx) / (sizeof(float) * 2), c);
}

void draw_down_shape(float x, float y, float size, ALLEGRO_COLOR c) {

	float r = size / 2.0;
	const float vtx[] = {				
		x - r, y - r,
		x - r, y + (r / 3.0f),
		x - (r / 3.0f), y + r,
		x + (r / 3.0f), y + r,
		x + r, y + (r / 3.0f),
		x + r, y - r,
	};
	al_draw_filled_polygon(vtx, sizeof(vtx) / (sizeof(float) * 2), c);
}

void draw_down_btn(float x, float y, float size, bool pressed) {

	int offset = pressed ? 1 : 2;
	draw_down_shape(x, y, size, CGA_BLUE);
	draw_down_shape(x - offset, y - offset, size, CGA_PINK);
}

void draw_up_btn(float x, float y, float size, bool pressed) {

	int offset = pressed ? 1 : 2;
	draw_up_shape(x, y, size, CGA_BLUE);
	draw_up_shape(x - offset, y - offset, size, CGA_PINK);
}

void GameStage::on_enter_stage() {
	
	PongClient* cl = static_cast<PongClient*>(this->engine);
	auto& game_handler = cl->get_game_handler();
	auto& touch_keys = this->engine->get_touch_keys();
	auto& audio = this->engine->get_audio_hnd();
	auto& allegro_hnd = this->engine->get_allegro_hnd();
	auto& cfg = this->engine->get_cfg();
	const bool show_buttons = cfg.sget<bool>("showButtons", true);

	touch_keys.clear_buttons();

	if (game_handler.pong_game->control_mode == CONTROLMODE_TWO_PLAYERS && game_handler.play_mode == PLAYMODE_LOCAL) {
		
		auto p1_up_btn = touch_keys.add_button(ALLEGRO_KEY_W, "");
		if (show_buttons) p1_up_btn->draw_fn = [](Button& btn) {
			int x = btn.get_x() + 30;
			int y = btn.get_y() + 40;
			int size = 30;
			draw_up_btn(x, y, size, btn.is_pressed());
		};

		auto p2_up_btn = touch_keys.add_button(ALLEGRO_KEY_UP, "");
		if (show_buttons) p2_up_btn->draw_fn = [&](Button& btn) {
			int x = btn.get_x() + btn.get_w() - 30;
			int y = btn.get_y() + 40;
			int size = 30;
			draw_up_btn(x, y, size, btn.is_pressed());
		};

		auto p1_down_btn = touch_keys.add_button(ALLEGRO_KEY_S, "");
		if (show_buttons) p1_down_btn->draw_fn = [&](Button& btn) {
			int x = btn.get_x() + 30;
			int y = btn.get_y() + btn.get_h() - 40;
			int size = 30;
			draw_down_btn(x, y, size, btn.is_pressed());
		};

		auto p2_down_btn = touch_keys.add_button(ALLEGRO_KEY_DOWN, "");
		if (show_buttons) p2_down_btn->draw_fn = [&](Button& btn) {
			int x = btn.get_x() + btn.get_w() - 30;
			int y = btn.get_y() + btn.get_h() - 40;
			int size = 30;
			draw_down_btn(x, y, size, btn.is_pressed());
		}; 

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

		auto down_btn = touch_keys.add_button(ALLEGRO_KEY_DOWN, "");
		if (show_buttons) down_btn->draw_fn = [&allegro_hnd](Button& btn) {
			float landscape_ratio = (float)allegro_hnd.get_window_height() / (float)allegro_hnd.get_window_width();
			int x = btn.get_x() + 30;
			int y = btn.get_y() + btn.get_h() / 2 + 40;
			int size = 30;
			if (landscape_ratio > 1) {
				x = btn.get_x() + 60;
				y = btn.get_y() + btn.get_h() - landscape_ratio * 50;
				size = 50;
			}
			draw_down_btn(x, y, size, btn.is_pressed());
		};

		auto up_btn = touch_keys.add_button(ALLEGRO_KEY_UP, "");
		if (show_buttons) up_btn->draw_fn = [&allegro_hnd](Button& btn) {
			float landscape_ratio = (float)allegro_hnd.get_window_height() / (float)allegro_hnd.get_window_width();
			int x = btn.get_x() + btn.get_w() - 30;
			int y = btn.get_y() + btn.get_h() / 2 - 40;
			int size = 30;
			if (landscape_ratio > 1) {
				x = btn.get_x() + btn.get_w() - 60;
				y = btn.get_y() + btn.get_h() - landscape_ratio * 50;
				size = 50;
			}
			draw_up_btn(x, y, size, btn.is_pressed());
		};

		touch_keys.fit_buttons(dp::client::ui::FIT_HORIZONTAL);

	}

	audio.create_and_play_sample({
		{ Do, 400 },
		{ Re, 200 },
		{ La, 100 },
		{ Si, 100 },
	});

	game_handler.pong_game->restart();
	game_handler.pong_game->iniciar_punto(1);

	delayer = 75;

}


void GameStage::on_event(ALLEGRO_EVENT evt) {

	PongClient* cl = static_cast<PongClient*>(this->engine);
	auto& game_handler = cl->get_game_handler();

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

				auto& conn = this->engine->get_io_client();
				conn.send_event("game/event", {
					{"type", "set_control_state"},
					{"state", new_st},
					{"control", control_p1},
					{"tick", game_handler.pong_game->tick}
				});

			}

		} else {
		
			game_handler.pong_game->players[0]->controls[control_p1] = new_st;
			game_handler.pong_game->players[1]->controls[control_p2] = new_st;
			
		} 

	}
	
}


void GameStage::process_message(string &msg) {
	
	auto& audio = this->engine->get_audio_hnd();

	if (msg == "scored") {
		
		audio.create_and_play_sample({
			{ Re, 130 },
			{ Do, 250 },
		});

	} else if (msg == "hit") {
		
		audio.create_and_play_sample({
			{ Mi, 40 },
		});

	}

}


void GameStage::trigger_desync() {
	
	this->engine->get_io_client().send_event("game/desync");

}


void GameStage::on_tick() {

	if (delayer > 0) {
		delayer--;
		return;
	}

	PongClient* cl = static_cast<PongClient*>(this->engine);
	auto& game_handler = cl->get_game_handler();

	if (game_handler.pong_game->paused) {
		return;
	}

	if (game_handler.play_mode == PLAYMODE_ONLINE) {

		try {

			PongClient* cl = static_cast<PongClient*>(this->engine);
			auto& controller = cl->get_groups_handler().get_online_game_controller();
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

		this->engine->set_stage(OVER);

		const uint8_t local_player_idx = game_handler.play_mode == PLAYMODE_ONLINE ? game_handler.local_player_idx : 0;
		const bool local_player_wins = local_player_idx == game_handler.pong_game->get_winner_idx();
		auto& audio = this->engine->get_audio_hnd();

		audio.prune(true);

		if (local_player_wins) {
			audio.create_and_play_sample({
				{ Do, 150, 3 },
				{ Mi, 150, 3 },
				{ Sol, 200, 3 },
				{ Do, 700, 4 },
			});
		} else {
			audio.create_and_play_sample({
				{ Re, 150, 3 },
				{ Re, 150, 3 },
				{ Re, 200, 3 },
				{ LaSos, 500, 2 },
			});
		}
		
	}
	
}


void GameStage::draw() {

	ALLEGRO_FONT* font = this->engine->get_font();
	float scale = this->engine->get_scale();
	auto& cfg = this->engine->get_cfg();
	const bool show_fps = cfg.sget<bool>("showFPS", false);
	const bool show_ping = cfg.sget<bool>("showPing", true);
	
	if (delayer > 0) {

		al_draw_textf(
			font, 
			CGA_PINK, 
			scale * this->engine->get_res_x() / 2, 
			scale * this->engine->get_res_y() / 2, 
			ALLEGRO_ALIGN_CENTER, 
			"%d", 
			1 + delayer / 25
		);
		
	} else {
		
		PongClient* cl = static_cast<PongClient*>(this->engine);
		auto& game_handler = cl->get_game_handler();

		if (game_handler.pong_game->paused) {

			al_draw_text(
				font, 
				CGA_PINK, 
				this->engine->get_res_x() * scale / 2, 
				this->engine->get_res_y() * scale / 2 - 5, 
				ALLEGRO_ALIGN_CENTER, 
				"PAUSE"
			);
			
		} else {

			//al_draw_text(font, RED, scale * DEF_W / 2, scale * 2, ALLEGRO_ALIGN_CENTER, "Press ESC to Main Menu");
			
			int secs = game_handler.pong_game->tick / 60;
			int secs_d = (secs % 60) / 10;
			int secs_u = secs % 10;
			int min = secs / 60;
			al_draw_textf(font, CGA_BLUE, scale * DEF_W / 2, 0, ALLEGRO_ALIGN_CENTER, "%d:%d%d", min, secs_d, secs_u);
				
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

			if (show_fps) {
				al_draw_textf(font, CGA_PINK, scale * 5, 0, ALLEGRO_ALIGN_LEFT, "FPS: %d", (int) (this->engine->get_fps()));
			}

			if (show_ping && game_handler.play_mode == PLAYMODE_ONLINE) {
				int ping = (int) (this->engine->get_io_client().get_ping_ms());
				al_draw_textf(font, CGA_PINK, scale * (DEF_W - 5), scale * 1, ALLEGRO_ALIGN_RIGHT, "PING: %d", ping);
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
		this->bonus_sprites[i] = engine->load_bitmap_resource(bonuses_defs[i].sprite_path);
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

