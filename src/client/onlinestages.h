#ifndef ONLINESTAGES_H
#define ONLINESTAGES_H

#include "hengine.h"
#include "mediatools.h"
#include "../ponggame.h"
#include <allegro5/allegro.h>
#include <boost/json.hpp>


class Controller { //GameTickSync (only for PLAYMODE_ONLINE)

	PongGame *game;

	void process_event(boost::json::object &evt);

	void sync_game(boost::json::object& evt);

public:

	void push_event(boost::json::object &evt);

	std::queue<boost::json::object> evt_queue;

	void setup(PongGame *game);

	void on_tick();

};

//-----------------------------------------------------------------------------

class ConnStage: public Stage {

	JC_TEXTINPUT* input;

	std::string server;

	bool start_connection = 0;

public:

	ConnStage(HGameEngine* _engine);

	void on_enter_stage();

	void on_event(ALLEGRO_EVENT event);
	
	void on_tick();

	void draw();

};

//-----------------------------------------------------------------------------

class LobbyStage: public Stage {

	JC_TEXTINPUT* input;

	bool ready = false;


public:

	LobbyStage(HGameEngine* _engine);

	void on_enter_stage();

	void on_event(ALLEGRO_EVENT event);
	
	void on_tick();

	void draw();

};

extern Controller controller;

#endif
