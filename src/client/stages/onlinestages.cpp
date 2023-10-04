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
		for(auto& m: members) {
			float y = 100 + 20 * i;
			al_draw_text(font, CGA_PINK, 20, y, ALLEGRO_ALIGN_LEFT, m.name.c_str());
			if (m.ready) {
				al_draw_text(font, CGA_BLUE, 150, y, ALLEGRO_ALIGN_LEFT, "READY!");
			}
			i++;
		}

	}

}
