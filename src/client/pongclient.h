#ifndef PONGCLIENT_H
#define PONGCLIENT_H

#include "../appinfo.h"
#include "pongnetgroupshandler.h"
#include <dp/client/baseclient.h>

#ifdef __APPLE__
#define RES_DIR "../Resources"
#else //if defined _WIN32 || defined _WIN64
#define RES_DIR "resources"
#endif

enum {
	MENU = 0,
	GAME,
	OVER,
	CONN,
	LOBBY,
	TUTO,
	CONF,
	MAX_SCREENS
};

class PongClient : public dp::client::BaseClient {

	PongNetGroupsHandler groups_handler;

public:

	PongClient() : BaseClient(APP_INFO, RES_DIR), groups_handler(this) {
		
		auto& cfg = this->get_cfg();
		std::string hostname = cfg.contains("serverHostname") ? cfg["serverHostname"].as_string().c_str() : "-" ;
		if (hostname == "copinstar.com") {
			// this is somenthing temporal
			this->set_cfg_param("serverHostname", "games.copinstar.com");
		}
		
	};

	dp::client::Stage* create_stage(uint16_t id);

	PongNetGroupsHandler& get_groups_handler() { return this->groups_handler; }

};

#endif
