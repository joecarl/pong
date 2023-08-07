#include "onlinestages.h"
#include "stages.h"
#include "mediatools.h"
#include "../utils.h"

#include <iostream>

using namespace std;

void Controller::push_event(boost::json::object &evt) {

	auto evt_type = evt["type"].as_string();

	if (evt_type == "sync") {
		
		cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
		cerr << "SYNCING: " << evt << endl;

		try {

			this->sync_game(evt["gamevars"].as_object());

		} catch (std::invalid_argument& e) {
			
			cerr << "SYNC ERROR:  " <<  e.what() << endl;

		}

	} else {
			
		//cout << "QUEUED: " << evt << endl;

		this->evt_queue.push(evt);

	}
	
}

void Controller::process_event(boost::json::object &evt) {

	cout << "Processing evt" << evt << endl;

	auto evt_type = evt["type"].as_string();
		
	if (evt_type == "set_control_state") {
		
		int control = evt["control"].as_int64();
		bool new_state = evt["state"].as_bool();
		int player_key = evt["player_key"].as_int64();
		
		this->game->players[player_key]->controls[control] = new_state;

	} else if (evt_type == "set_paused_state") {

		bool new_state = evt["state"].as_bool();
		this->game->paused = new_state;

	} else {

		cerr << "Unknown event type: " << evt_type << endl;

	}

}


void syncPlayer(PlayerP* p, boost::json::object& vars) {

	p->x = vars["x"].as_int64();
	p->y = vars["y"].as_int64();
	p->com_txt_y = vars["com_txt_y"].as_int64();

	p->score = vars["score"].as_int64();
	p->medlen = vars["medlen"].as_int64();
	p->racha = vars["racha"].as_int64();

	unsigned int i;
	i = 0;
	for (auto &v: vars["bonus_timers"].as_array()) {
		p->bonus_timers[i++] = boost::json::value_to<unsigned int>(v);
	}

	i = 0;
	for (auto &v: vars["controls"].as_array()) {
		p->controls[i++] = v.as_bool();
	}
	
}

void syncElement(Element* e, boost::json::object& vars) {

	e->stat = vars["stat"].as_bool();
	e->x = vars["x"].as_double();
	e->y = vars["y"].as_double();
	e->x00 = vars["x00"].as_double();
	e->y00 = vars["y00"].as_double();
	e->radius = vars["radius"].as_double();
	e->vx = vars["vx"].as_double();
	e->vy = vars["vy"].as_double();
	e->t = vars["t"].as_double();

}

void syncBall(Ball* b, boost::json::object& vars) {
	
	syncElement((Element*) b, vars);
	
}

void syncBonus(Bonus* b, boost::json::object& vars) {

	syncElement((Element*) b, vars);
	b->cooldown = vars["cooldown"].as_int64();
	
}


void Controller::sync_game(boost::json::object& vars) {

	this->game->tick = boost::json::value_to<unsigned int>(vars["tick"]);
	this->game->paused = vars["paused"].as_bool();

	cout << "sync players ..." << endl;
	syncPlayer(this->game->players[0], vars["p0vars"].as_object());
	syncPlayer(this->game->players[1], vars["p1vars"].as_object());
	
	cout << "sync bonus ..." << endl;
	syncBonus(this->game->bonus[0], vars["bonus0vars"].as_object());
	syncBonus(this->game->bonus[1], vars["bonus1vars"].as_object());

	cout << "sync ball ..." << endl;
	syncBall(this->game->ball, vars["ballvars"].as_object());

}



void Controller::setup(PongGame *game) {
	
	this->game = game;
	
	//vaciamos la cola de eventos
	std::queue<boost::json::object> empty;
	std::swap(this->evt_queue, empty);

}


void Controller::on_tick() {

	while (this->evt_queue.size() > 0) {

		auto evt = this->evt_queue.front();
		auto evt_tick = (unsigned int) evt["tick"].as_int64();

		if (evt_tick == this->game->tick) {

			this->process_event(evt);

			this->evt_queue.pop();

		} else if (evt_tick < this->game->tick) {

			cerr << "DESYNC! evt_tick: " << evt_tick << " | gameTick: " << this->game->tick << endl;

			this->evt_queue.pop();
		
			throw std::runtime_error("Evento perdido");

		} else {
			
			break;

		}

	}
}


Controller controller;



//-----------------------------------------------------------------------------
//------------------------------- [ ConnStage ] -------------------------------


ConnStage::ConnStage(HGameEngine* _engine): Stage(_engine) {

	this->input = new JC_TEXTINPUT(this->engine->font);

}

void ConnStage::on_enter_stage() {

	input->start();

	this->engine->touch_keys.clear_buttons();
	this->engine->touch_keys.add_button(ALLEGRO_KEY_ENTER, "Enter");
	this->engine->touch_keys.add_button(ALLEGRO_KEY_ESCAPE, "Esc");
	this->engine->touch_keys.fit_buttons(FIT_BOTTOM, 10);

}

void ConnStage::on_event(ALLEGRO_EVENT event) {

	if (event.type == ALLEGRO_EVENT_KEY_CHAR) {

		if (input->active) {
			
			if (event.keyboard.keycode != ALLEGRO_KEY_ENTER) {

				input->process_key(event.keyboard.unichar, event.keyboard.keycode);

			}

		}

	} else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {

		int keycode = event.keyboard.keycode;

		if (keycode == ALLEGRO_KEY_ESCAPE) {//ESC (SALIR)

			al_rest(0.2);
			input->active = false;
			this->engine->set_stage(MENU);

		} else if (keycode == ALLEGRO_KEY_ENTER) {

			input->finish();
			server = input->get_value();

			if (server.empty()) {

				server = this->engine->cfg["defaultServer"].as_string().c_str();

			}

			auto port = (unsigned short) this->engine->cfg["defaultPort"].as_int64();
						
			this->engine->connection.connect(server, port);
			
		}

	}

}

void ConnStage::on_tick() {

	int conn_state = this->engine->connection.get_state();

	if (conn_state == CONNECTION_STATE_CONNECTED_FULL) {

		this->engine->set_stage(LOBBY);
		
	}

}

void ConnStage::draw() {

	ALLEGRO_FONT* font = this->engine->font;

	IoClient* connection = &this->engine->connection;

	string pts = get_wait_string();
	
	al_draw_text(font, WHITE, 20, 30, ALLEGRO_ALIGN_LEFT, "ENTER SERVER IP ADDRESS or press ");
	al_draw_text(font, WHITE, 20, 40, ALLEGRO_ALIGN_LEFT, "enter to connect to default server:");

	if (input->active) {

		input->draw(30, 60);

	} else {

		int conn_state = connection->get_state();

		if (conn_state == CONNECTION_STATE_CONNECTING) {

			al_draw_textf(font, WHITE, 20, 60, ALLEGRO_ALIGN_LEFT, "Trying %s %s", server.c_str(), pts.c_str());
				
		} else if (conn_state == CONNECTION_STATE_DISCONNECTED) {
			
			//al_draw_text(font, WHITE, 20, 60, ALLEGRO_ALIGN_LEFT, "Connection error.");
			al_draw_textf(font, WHITE, 20, 60, ALLEGRO_ALIGN_LEFT, "Error: could not connect to %s", server.c_str());
			
		} else if (conn_state > CONNECTION_STATE_CONNECTED) {

			al_draw_textf(font, WHITE, 20, 60, ALLEGRO_ALIGN_LEFT, "Connected to %s", server.c_str());
			al_draw_textf(font, WHITE, 20, 75, ALLEGRO_ALIGN_LEFT, "Please wait %s", pts.c_str());
		
		}

	}

}



//-----------------------------------------------------------------------------
//------------------------------- [ LobbyStage ] ------------------------------


LobbyStage::LobbyStage(HGameEngine* _engine): Stage(_engine) {

	this->input = new JC_TEXTINPUT(this->engine->font);

}


void LobbyStage::on_enter_stage() {

	
	this->engine->touch_keys.clear_buttons();
	this->engine->touch_keys.add_button(ALLEGRO_KEY_ENTER, "Enter");
	this->engine->touch_keys.add_button(ALLEGRO_KEY_ESCAPE, "Esc");
	this->engine->touch_keys.fit_buttons(FIT_BOTTOM, 10);

	this->ready = false;

	this->engine->connection.set_process_actions_fn([&] (boost::json::object& evt) {

		cout << "RECEIVED: " << evt << endl;
		if (evt["type"] == "game_start") {

			game_handler.make_new_pong_game((int_fast32_t) evt["seed"].as_int64());

			controller.setup(game_handler.pong_game);

			this->engine->set_stage(GAME);
		
		}

	});

}


void LobbyStage::on_event(ALLEGRO_EVENT event) {

	 if (event.type == ALLEGRO_EVENT_KEY_DOWN) {

		int keycode = event.keyboard.keycode;

		if (keycode == ALLEGRO_KEY_ENTER) {
					
			boost::json::object pkg = {{"type", "ready_to_play"}}; //play_again
			
			this->engine->connection.qsend(boost::json::serialize(pkg));

			this->ready = true;

		} else if (keycode == ALLEGRO_KEY_ESCAPE) {
			
			this->engine->set_stage(MENU);

		}

	}

}


void LobbyStage::on_tick() {

}


void LobbyStage::draw() {

	ALLEGRO_FONT* font = this->engine->font;

	IoClient* connection = &this->engine->connection;

	al_draw_text(font, WHITE, 20, 30, ALLEGRO_ALIGN_LEFT, "LOBBY");

	int conn_state = connection->get_state();

	if (conn_state == CONNECTION_STATE_CONNECTED_FULL) {

		al_draw_textf(font, WHITE, 20, 60, ALLEGRO_ALIGN_LEFT, "Connected to %s", connection->current_host.c_str());

	}
	
	if (this->ready) {

		string pts = get_wait_string();
	
		al_draw_textf(font, WHITE, 20, 75, ALLEGRO_ALIGN_LEFT, "Ok. Wait please %s", pts.c_str());
	
	} else {

		al_draw_textf(font, WHITE, 20, 75, ALLEGRO_ALIGN_LEFT, "Press Enter when you are ready");
	
	}

}
