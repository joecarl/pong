#ifndef PONGNETGROUPSHANDLER_H
#define PONGNETGROUPSHANDLER_H

#include <dp/client/netgroupshandler.h>
#include <dp/client/onlinegamecontroller.h>
#include <dp/neteventslistenershandler.h>
#include <queue>

class PongClient;


class PongNetGroupsHandler: public dp::client::NetGroupsHandler {

	dp::client::OnlineGameController controller;

public:

	PongNetGroupsHandler(PongClient* client);
	
	void create_group(dp::client::Connection* net, std::string id, std::string owner_id, boost::json::array& members);

	dp::client::OnlineGameController& get_online_game_controller() { return this->controller; }

};

#endif