
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

	PongGame game;

	std::vector<Client*> clients;

	void game_main_loop(boost::asio::steady_timer* t);

public:

	Group();

	void addClient(Client* cl);

	void sendToAll(std::string pkg);

	void startGame();

};

#endif