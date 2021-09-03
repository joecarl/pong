#ifndef ONLINESTAGES_H
#define ONLINESTAGES_H

#include "hengine.h"
#include "mediatools.h"
#include "../classes.h"
#include <allegro5/allegro.h>
#include <boost/json.hpp>


class Controller{ //GameTickSync (only for PLAYMODE_ONLINE)

	PongGame *game;

	void process_event(boost::json::object &evt);

public:

	std::queue<boost::json::object> evt_queue;

	void setup(PongGame *game);

	void onTick();

};

//-----------------------------------------------------------------------------

class ConnStage: public Stage{

	JC_TEXTINPUT* input;

	std::string server;

	bool start_connection = 0;

public:

	ConnStage(HGameEngine* _engine);

	void onEnterStage();

	void onEvent(ALLEGRO_EVENT event);
	
	void onTick();

	void draw();

};

//-----------------------------------------------------------------------------

class LobbyStage: public Stage{

	JC_TEXTINPUT* input;

	bool ready;


public:

	LobbyStage(HGameEngine* _engine);

	void onEnterStage();

	void onEvent(ALLEGRO_EVENT event);
	
	void onTick();

	void draw();

};

extern Controller controller;

#endif
