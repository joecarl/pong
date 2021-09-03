
#ifndef group_hpp
#define group_hpp

#include "../classes.h"
#include "server.h"

#include <ctime>
#include <vector>
#include <iostream>
#include <stdlib.h>

#include <boost/json.hpp>
#include <boost/chrono.hpp>

class Group{

	PongGame *game = nullptr;

	std::vector<Client*> clients;

	std::queue<boost::json::object> evt_queue;

	boost::asio::io_context *io;

	boost::asio::steady_timer *t;

	int players_ready;

	void game_main_loop();

	void process_event(boost::json::object &evt);

public:

	Group();
	
	void newGame();

	void addClient(Client* cl);

	void sendToAll(std::string pkg);

	void startGame();

};

#endif