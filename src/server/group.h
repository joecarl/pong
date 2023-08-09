#ifndef GROUP_H
#define GROUP_H

#include "../ponggame.h"
#include "server.h"

#include <ctime>
#include <vector>
#include <iostream>
#include <stdlib.h>

#include <boost/json.hpp>
#include <boost/chrono.hpp>

class Group {

	PongGame *game = nullptr;

	std::vector<Client*> clients;

	std::queue<boost::json::object> evt_queue;

	boost::asio::io_context *io;

	boost::asio::steady_timer *t;

	bool players_ready[2];

	void game_main_loop();

	void process_event(boost::json::object &evt);

public:

	Group();
	
	void new_game();

	void add_client(Client* cl);

	void send_to_all(const std::string& pkg);

	void start_game();

};

#endif
