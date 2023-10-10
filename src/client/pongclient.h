#ifndef PONGCLIENT_H
#define PONGCLIENT_H

#include "../appinfo.h"
#include "../ponggame.h"
#include "pongnetgroupshandler.h"
#include <dp/client/baseclient.h>

#ifdef __APPLE__
#define RES_DIR "../Resources"
#else //if defined _WIN32 || defined _WIN64
#define RES_DIR "resources"
#endif

enum {
	PLAYMODE_NONE = 0,
	PLAYMODE_LOCAL,
	PLAYMODE_ONLINE
};


class GameHandler {

	int control_mode = CONTROLMODE_NONE;

	/**
	 * Current game's players names (online mode only)
	 */
	std::string players_names[2];

public:

	int play_mode = PLAYMODE_NONE;
	
	/**
	 * Current game's local player index
	 */
	uint8_t local_player_idx = 0;

	PongGame* pong_game = nullptr;

	~GameHandler();

	void setup(int play_mode, int control_mode);

	void make_new_pong_game(int_fast32_t seed);
	
	int get_control(int k_code, uint8_t player_idx);

	/**
	 * Sets the corresponding player name to show in UI
	 */
	void set_player_name(uint8_t player_idx, const std::string& name);

	/**
	 * Retrieves the corresponding player name
	 */
	std::string get_player_name(uint8_t player_idx);

	void cleanup();

};


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

	GameHandler game_handler;

public:

	PongClient() : BaseClient(APP_INFO, RES_DIR), groups_handler(this) {
		
		auto& cfg = this->get_cfg();
		std::string hostname = cfg.sget<std::string>("serverHostname", "-");
		if (hostname == "copinstar.com") {
			// this is somenthing temporal
			this->set_cfg_param("serverHostname", "games.copinstar.com");
		}
		
	};

	dp::client::Stage* create_stage(uint16_t id);

	PongNetGroupsHandler& get_groups_handler() { return this->groups_handler; }

	GameHandler& get_game_handler() { return this->game_handler; }

};

#endif
