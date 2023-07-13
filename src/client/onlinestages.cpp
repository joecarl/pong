#include "onlinestages.h"
#include "stages.h"
#include "mediatools.h"
#include "../utils.h"

#include <iostream>

using namespace std;

void Controller::push_event(boost::json::object &evt) {

	auto evtType = evt["type"].as_string();

	if (evtType == "sync") {

		
		cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
		cout << "SYNCING: " << evt << endl;

		try {

			this->syncGame(evt["gamevars"].as_object());

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

	auto evtType = evt["type"].as_string();
		
	if (evtType == "set_control_state") {
		
		int control = evt["control"].as_int64();
		bool newState = evt["state"].as_bool();
		int playerID = evt["playerKey"].as_int64();
		
		this->game->players[playerID]->controls[control] = newState;

	} else if (evtType == "set_paused_state") {

		bool newState = evt["state"].as_bool();
		this->game->paused = newState;

	} else {

		cerr << "Unknown event type: " << evtType << endl;

	}

}


void syncPlayer(PlayerP* p, boost::json::object& vars) {

	p->x = vars["x"].as_int64();
	p->y = vars["y"].as_int64();
	p->comTxtY = vars["comTxtY"].as_int64();

	//std::string comTxt;

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
	e->vX = vars["vX"].as_double();
	e->vY = vars["vY"].as_double();
	e->t = vars["t"].as_double();

}

void syncBall(Ball* b, boost::json::object& vars) {
	
	syncElement((Element*) b, vars);
	
}

void syncBonus(Bonus* b, boost::json::object& vars) {

	syncElement((Element*) b, vars);
	b->cooldown = vars["cooldown"].as_int64();
	
}


void Controller::syncGame(boost::json::object& vars) {

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
	std::swap( this->evt_queue, empty );

}


void Controller::onTick() {

	while (this->evt_queue.size() > 0) {

		auto evt = this->evt_queue.front();
		auto evtTick = (unsigned int)evt["tick"].as_int64();

		if (evtTick == this->game->tick) {

			this->process_event(evt);

			this->evt_queue.pop();

		} else if (evtTick < this->game->tick) {

			cerr << "DESYNC! evtTick: " << evtTick << " | gameTick: " << this->game->tick << endl;

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


ConnStage::ConnStage(HGameEngine* _engine):Stage(_engine) {

	this->input = new JC_TEXTINPUT(this->engine->font);

}

void ConnStage::onEnterStage() {

	input->start();

	this->engine->touchKeys.clearButtons();
	this->engine->touchKeys.addButton(ALLEGRO_KEY_ENTER, "Enter");
	this->engine->touchKeys.addButton(ALLEGRO_KEY_ESCAPE, "Esc");
	this->engine->touchKeys.fitButtons(FIT_BOTTOM, 10);

}

void ConnStage::onEvent(ALLEGRO_EVENT event) {

	if (event.type == ALLEGRO_EVENT_KEY_CHAR) {

		if (input->active) {
			
			if (event.keyboard.keycode != ALLEGRO_KEY_ENTER) {

				input->processKey(event.keyboard.unichar, event.keyboard.keycode);

			}

		}

	} else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {

		int keycode = event.keyboard.keycode;

		if (keycode == ALLEGRO_KEY_ESCAPE) {//ESC (SALIR)

			al_rest(0.2);
			input->active = false;
			this->engine->setStage(MENU);

		} else if (keycode == ALLEGRO_KEY_ENTER) {

			input->finish();
			server = input->getValue();

			if (server.empty()) {

				server = this->engine->cfg["defaultServer"].as_string().c_str();

			}

			auto port = (unsigned short) this->engine->cfg["defaultPort"].as_int64();
						
			this->engine->connection.connect(server, port);
			
		}

	}

}

void ConnStage::onTick() {

	int connState = this->engine->connection.get_state();

	if (connState == CONNECTION_STATE_CONNECTED) {

		this->engine->setStage(LOBBY);
		
	}

}

void ConnStage::draw() {

	ALLEGRO_FONT* font = this->engine->font;

	IoClient* connection = &this->engine->connection;

	string pts = GetWaitString();
	
	al_draw_text(font, WHITE, 20, 30, ALLEGRO_ALIGN_LEFT, "ENTER SERVER IP ADDRESS or press ");
	al_draw_text(font, WHITE, 20, 40, ALLEGRO_ALIGN_LEFT, "enter to connect to default server:");

	if (input->active) {

		input->draw(30, 60);

	} else {

		int connState = connection->get_state();

		if (connState == CONNECTION_STATE_CONNECTING) {

			al_draw_textf(font, WHITE, 20, 60, ALLEGRO_ALIGN_LEFT, "Trying %s %s", server.c_str(), pts.c_str());
				
		} else if (connState == CONNECTION_STATE_DISCONNECTED) {
			
			//al_draw_text(font, WHITE, 20, 60, ALLEGRO_ALIGN_LEFT, "Connection error.");
			al_draw_textf(font, WHITE, 20, 60, ALLEGRO_ALIGN_LEFT, "Error: could not connect to %s", server.c_str());
			
		} else if (connState == CONNECTION_STATE_CONNECTED) {

			al_draw_textf(font, WHITE, 20, 60, ALLEGRO_ALIGN_LEFT, "Connected to %s", server.c_str());
			al_draw_textf(font, WHITE, 20, 75, ALLEGRO_ALIGN_LEFT, "Please wait %s", pts.c_str());
		
		}

	}

}



//-----------------------------------------------------------------------------
//------------------------------- [ LobbyStage ] ------------------------------


LobbyStage::LobbyStage(HGameEngine* _engine):Stage(_engine) {

	this->input = new JC_TEXTINPUT(this->engine->font);

}


void LobbyStage::onEnterStage() {

	
	this->engine->touchKeys.clearButtons();
	this->engine->touchKeys.addButton(ALLEGRO_KEY_ENTER, "Enter");
	this->engine->touchKeys.addButton(ALLEGRO_KEY_ESCAPE, "Esc");
	this->engine->touchKeys.fitButtons(FIT_BOTTOM, 10);

	this->ready = false;

	this->engine->connection.process_actions_fn = [&](boost::json::object& evt) {

		cout << "RECEIVED: " << evt << endl;
		if (evt["type"] == "game_start") {

			gameHandler.makeNewPongGame((int_fast32_t)evt["seed"].as_int64());

			controller.setup(gameHandler.pongGame);

			this->engine->setStage(GAME);
		
		}

	};

}


void LobbyStage::onEvent(ALLEGRO_EVENT event) {

	 if (event.type == ALLEGRO_EVENT_KEY_DOWN) {

		int keycode = event.keyboard.keycode;

		if (keycode == ALLEGRO_KEY_ENTER) {
					
			boost::json::object pkg = {{"type", "ready_to_play"}}; //play_again
			
			this->engine->connection.qsend(boost::json::serialize(pkg));

			this->ready = true;

		} else if (keycode == ALLEGRO_KEY_ESCAPE) {
			
			this->engine->setStage(MENU);

		}

	}

}


void LobbyStage::onTick() {

}


void LobbyStage::draw() {

	ALLEGRO_FONT* font = this->engine->font;

	IoClient* connection = &this->engine->connection;

	al_draw_text(font, WHITE, 20, 30, ALLEGRO_ALIGN_LEFT, "LOBBY");

	int connState = connection->get_state();

	if (connState == CONNECTION_STATE_CONNECTED) {

		al_draw_textf(font, WHITE, 20, 60, ALLEGRO_ALIGN_LEFT, "Connected to %s", connection->current_host.c_str());

	}
	
	if (this->ready) {

		string pts = GetWaitString();
	
		al_draw_textf(font, WHITE, 20, 75, ALLEGRO_ALIGN_LEFT, "Ok. Wait please %s", pts.c_str());
	
	} else {

		al_draw_textf(font, WHITE, 20, 75, ALLEGRO_ALIGN_LEFT, "Press Enter when you are ready");
	
	}

}
