#ifndef ONLINESTAGES_H
#define ONLINESTAGES_H

#include <dp/client/stage.h>
#include <allegro5/allegro.h>


class ConnStage: public dp::client::Stage {

	bool start_connection = 0;

public:

	ConnStage(dp::client::BaseClient* _engine);

	void on_enter_stage();

	void on_event(ALLEGRO_EVENT event);
	
	void on_tick();

	void draw();

};

//-----------------------------------------------------------------------------

class LobbyStage: public dp::client::Stage {

public:

	LobbyStage(dp::client::BaseClient* _engine);

	void on_enter_stage();

	void on_event(ALLEGRO_EVENT event);
	
	void on_tick();

	void draw();

};

#endif
