#ifndef PONGSERVER_H
#define PONGSERVER_H

#include "../ponggame.h"
#include "../appinfo.h"
#include <dp/serendipia.h>
#include <dp/server/baseserver.h>
#include <dp/server/group.h>

class PongServer : public dp::server::BaseServer {

public:
	
	PongServer(uint16_t _port) : BaseServer(APP_INFO, _port) { };

	dp::BaseGame* create_game();

	boost::json::object export_game(dp::BaseGame* game);

};

#endif
