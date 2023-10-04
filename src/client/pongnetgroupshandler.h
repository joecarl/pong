#ifndef PONGNETGROUPSHANDLER_H
#define PONGNETGROUPSHANDLER_H

#include <dp/client/netgroupshandler.h>
#include <dp/neteventslistenershandler.h>
#include <queue>

class PongClient;
class PongGame;

class OnlineGameController { //GameTickSync (only for PLAYMODE_ONLINE)

	PongGame *game;

	void process_event(boost::json::object &evt);

	void sync_game(boost::json::object& evt);

public:

	void push_event(boost::json::object &evt);

	std::queue<boost::json::object> evt_queue;

	void setup(PongGame *game);

	void on_tick();

};

class PongNetGroupsHandler: public dp::client::NetGroupsHandler {

	OnlineGameController controller;

public:

	PongNetGroupsHandler(PongClient* client);
	
	void create_group(dp::client::Connection* net, std::string id, std::string owner_id, boost::json::array& members);

	OnlineGameController& get_online_game_controller() { return this->controller; }

};

#endif