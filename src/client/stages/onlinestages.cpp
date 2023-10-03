#include "onlinestages.h"
#include "stages.h"
#include "../pongclient.h"
#include <dp/client/mediatools.h>
#include <dp/utils.h>

#include <iostream>

using std::string;
using std::cout;
using std::cerr;
using std::endl;

void Controller::push_event(boost::json::object &evt) {

	auto evt_type = evt["type"].as_string();

	if (evt_type == "sync") {
		
		cerr << "!! SYNCING: " << evt << endl;

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

	//cout << "Processing evt" << evt << endl;

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


void sync_player(PlayerP* p, boost::json::object& vars) {

	p->x = vars["x"].as_int64();
	p->y = vars["y"].as_int64();
	//p->com_txt_y = vars["com_txt_y"].as_int64(); //this var is only processed in the client

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

void sync_element(Element* e, boost::json::object& vars) {

	e->stat = vars["stat"].as_bool();
	e->x = vars["x"].as_double();
	e->y = vars["y"].as_double();
	e->radius = vars["radius"].as_double();
	e->vx = vars["vx"].as_double();
	e->vy = vars["vy"].as_double();

}

void sync_wall(Wall* w, boost::json::object& vars) {
	
	sync_element((Element*) w, vars);
	
}

void sync_ball(Ball* b, boost::json::object& vars) {
	
	sync_element((Element*) b, vars);
	
}

void sync_bonus(Bonus* b, boost::json::object& vars) {

	sync_element((Element*) b, vars);
	b->cooldown = vars["cooldown"].as_int64();
	
}


void Controller::sync_game(boost::json::object& vars) {

	this->game->tick = boost::json::value_to<unsigned int>(vars["tick"]);
	this->game->warmup = boost::json::value_to<uint16_t>(vars["warmup"]);
	this->game->paused = vars["paused"].as_bool();
	this->game->rnd.index = vars["rnd_index"].as_int64();

	cout << "sync players ..." << endl;
	sync_player(this->game->players[0], vars["p0vars"].as_object());
	sync_player(this->game->players[1], vars["p1vars"].as_object());
	
	cout << "sync bonus ..." << endl;
	for (uint8_t i = 0; i < BONUS_MAX; i++) {
		sync_bonus(this->game->bonus[i], vars["bonus"].as_array()[i].as_object());
	}

	cout << "sync walls ..." << endl;
	for (uint8_t i = 0; i < 4; i++) {
		sync_wall(this->game->walls[i], vars["walls"].as_array()[i].as_object());
	}

	cout << "sync ball ..." << endl;
	sync_ball(this->game->ball, vars["ballvars"].as_object());

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

			cerr << "DESYNC! evt_tick: " << evt_tick << " | game_tick: " << this->game->tick << endl;

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


ConnStage::ConnStage(dp::client::BaseClient* _engine) : Stage(_engine) {

}

void ConnStage::on_enter_stage() {


	auto& touch_keys = this->engine->get_touch_keys();

	touch_keys.clear_buttons();
	touch_keys.add_button(ALLEGRO_KEY_ENTER, "Enter");
	touch_keys.add_button(ALLEGRO_KEY_ESCAPE, "Esc");
	touch_keys.fit_buttons(dp::client::ui::FIT_BOTTOM, 10);

}

void ConnStage::on_event(ALLEGRO_EVENT event) {

	if (event.type == ALLEGRO_EVENT_KEY_DOWN) {

		int keycode = event.keyboard.keycode;

		auto& cfg = this->engine->get_cfg();

		if (keycode == ALLEGRO_KEY_ESCAPE) {//ESC (SALIR)

			al_rest(0.2);
			this->engine->set_stage(MENU);

		} else if (keycode == ALLEGRO_KEY_ENTER) {
			
			string server = cfg["serverHostname"].as_string().c_str();

			auto port = (unsigned short) cfg["serverPort"].as_int64();
						
			this->engine->get_io_client().connect(server, port);

			start_connection = true;
			
		}

	}

}

void ConnStage::on_tick() {

	auto& connection = this->engine->get_io_client();

	int conn_state = connection.get_state();

	if (conn_state == dp::CONNECTION_STATE_CONNECTED_FULL) {

		this->engine->set_stage(LOBBY);
		
	}

}

void ConnStage::draw() {

	ALLEGRO_FONT* font = this->engine->get_font();
	auto& cfg = this->engine->get_cfg();
	auto& connection = this->engine->get_io_client();

	string server = cfg["serverHostname"].as_string().c_str();
	string pts = dp::get_wait_string();

	const float x_offset = 30;

	al_draw_text(font, WHITE, x_offset, 30, ALLEGRO_ALIGN_LEFT, "Press enter to connect to server");

	//al_draw_text(font, WHITE, 20, 30, ALLEGRO_ALIGN_LEFT, "ENTER SERVER IP ADDRESS or press ");
	//al_draw_text(font, WHITE, 20, 40, ALLEGRO_ALIGN_LEFT, "enter to connect to default server:");

	if (!start_connection) {

		al_draw_text(font, WHITE, x_offset, 45, ALLEGRO_ALIGN_LEFT, server.c_str());

	} else {

		int conn_state = connection.get_state();

		if (conn_state == dp::CONNECTION_STATE_CONNECTING) {

			al_draw_textf(font, WHITE, x_offset, 60, ALLEGRO_ALIGN_LEFT, "Trying %s %s", server.c_str(), pts.c_str());
				
		} else if (conn_state == dp::CONNECTION_STATE_DISCONNECTED) {
			
			//al_draw_text(font, WHITE, x_offset, 60, ALLEGRO_ALIGN_LEFT, "Connection error.");
			al_draw_textf(font, WHITE, x_offset, 60, ALLEGRO_ALIGN_LEFT, "Error: could not connect to %s", server.c_str());
			
		} else if (conn_state > dp::CONNECTION_STATE_CONNECTED) {

			al_draw_textf(font, WHITE, x_offset, 60, ALLEGRO_ALIGN_LEFT, "Connected to %s", server.c_str());
			al_draw_textf(font, WHITE, x_offset, 75, ALLEGRO_ALIGN_LEFT, "Please wait %s", pts.c_str());
		
		}

	}

}



//-----------------------------------------------------------------------------
//------------------------------- [ LobbyStage ] ------------------------------
// Quiza deberia llamarse server stage, ya que la idea es que también gestione
// el momento anterior a unirse a un grupo, configuracion de partida, etc... 


LobbyStage::LobbyStage(dp::client::BaseClient* _engine) : 
	Stage(_engine)
{

}


void LobbyStage::on_enter_stage() {

	auto& connection = this->engine->get_io_client();
	/*
	boost::json::object pkg = {
		{"type", "join_request"},
		{"group_id", ""},

	};
	*/
	boost::json::object pkg = {
		{"type", "clientConfig"},
		{"data", this->engine->get_cfg()},
	};
	connection.qsend(boost::json::serialize(pkg));


	auto& touch_keys = this->engine->get_touch_keys();
	
	touch_keys.clear_buttons();
	touch_keys.add_button(ALLEGRO_KEY_ENTER, "Enter");
	touch_keys.add_button(ALLEGRO_KEY_ESCAPE, "Esc");
	touch_keys.fit_buttons(dp::client::ui::FIT_BOTTOM, 10);


}


void LobbyStage::on_event(ALLEGRO_EVENT event) {

	//auto& io_client = this->engine->get_io_client();

	if (event.type == ALLEGRO_EVENT_KEY_DOWN) {

		int keycode = event.keyboard.keycode;

		if (keycode == ALLEGRO_KEY_ENTER) {

			auto client = static_cast<PongClient*>(this->engine);

			auto& gh = client->get_groups_handler();
			auto g = gh.get_current_group();
			if (g == nullptr) {
				return;
			}
			g->send_ready_state(true);

		} else if (keycode == ALLEGRO_KEY_ESCAPE) {
			
			this->engine->set_stage(MENU);

		}

	}

}


void LobbyStage::on_tick() {

}


void LobbyStage::draw() {

	ALLEGRO_FONT* font = this->engine->get_font();
	auto& conn = this->engine->get_io_client();

	al_draw_text(font, WHITE, 20, 30, ALLEGRO_ALIGN_LEFT, "LOBBY");

	int conn_state = conn.get_state();
	if (conn_state == dp::CONNECTION_STATE_CONNECTED_FULL) {

		al_draw_textf(font, CGA_BLUE, 20, 60, ALLEGRO_ALIGN_LEFT, "Connected to %s", conn.get_current_host().c_str());

	}

	auto client = static_cast<PongClient*>(this->engine);
	auto& gh = client->get_groups_handler();
	auto g = gh.get_current_group();
	if (g == nullptr) {
		
		string pts = dp::get_wait_string();
		al_draw_textf(font, CGA_BLUE, 20, 75, ALLEGRO_ALIGN_LEFT, "Joining group %s", pts.c_str());

	} else {

		string local_id = conn.get_local_id();
		auto m = g->get_member_info(local_id);
		bool ready = m != nullptr ? m->ready : false;
		if (!ready) {

			al_draw_textf(font, WHITE, 20, 75, ALLEGRO_ALIGN_LEFT, "Press Enter when you are ready");
		
		} else {

			string pts = dp::get_wait_string();
			al_draw_textf(font, WHITE, 20, 75, ALLEGRO_ALIGN_LEFT, "Ok. Please wait %s", pts.c_str());
		
		}

		auto& members = g->get_members();
		uint8_t i = 0;
		for(auto& m_iter: members) {
			auto& m = m_iter.second;
			float y = 100 + 20 * i;
			al_draw_text(font, CGA_PINK, 20, y, ALLEGRO_ALIGN_LEFT, m.name.c_str());
			if (m.ready) {
				al_draw_text(font, CGA_BLUE, 150, y, ALLEGRO_ALIGN_LEFT, "READY!");
			}
			i++;
		}

	}

}
