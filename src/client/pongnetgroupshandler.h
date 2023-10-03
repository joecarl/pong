#ifndef PONGNETGROUPSHANDLER_H
#define PONGNETGROUPSHANDLER_H

#include <dp/client/netgroupshandler.h>
#include <dp/neteventslistenershandler.h>

class PongClient;

class PongNetGroupsHandler: public dp::client::NetGroupsHandler {

public:

	PongNetGroupsHandler(PongClient* client);
	
	void create_group(dp::client::Connection* net, std::string id, std::string owner_id, boost::json::array& members);

};

#endif