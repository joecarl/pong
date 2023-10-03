#include "pongnetgroupshandler.h"
#include "pongclient.h"
#include "stages/onlinestages.h" //controller
#include "stages/stages.h" //game_handler

PongNetGroupsHandler::PongNetGroupsHandler(PongClient* client) : 
	dp::client::NetGroupsHandler(client)
{
	this->nelh->add_event_listener("net/set_client_id", [this] (boost::json::object& data) {
		auto cl = this->get_client();
		cl->get_io_client().send_event("client/login", { {"cfg", cl->get_cfg()} });
	});
}


void PongNetGroupsHandler::create_group(dp::client::Connection* net, std::string id, std::string owner_id, boost::json::array& members) {
	
	NetGroupsHandler::create_group(net, id, owner_id, members);
	
	auto local_id = net->get_local_id();
	auto group_nelh = this->group->get_nelh();
	group_nelh->add_event_listener("group/game_start", [this, local_id] (boost::json::object& data) {

		game_handler.make_new_pong_game((int_fast32_t) data["seed"].as_int64());
		controller.setup(game_handler.pong_game);

		auto players_order = data["players_order"].as_array(); //quiza podria omitirse y utilizar el mismo orden que haya en el grupo, pero no me fio de que se trafuque
		uint8_t i = 0;
		for (auto& ord: players_order) {
			std::string client_id = ord.as_string().c_str();
			auto info = this->group->get_member_info(client_id);
			if (client_id == local_id) {
				game_handler.local_player_idx = i;
			}
			//cout << "curioso " << player_cfg << endl;
			game_handler.set_player_name(i, info->name);
			i++;
		}
		
		PongClient* client = static_cast<PongClient*>(this->get_client());
		client->set_stage(GAME);
	
	});

	// TODO: ver bien como gestionar esto: (quiza la clase controller puede ir a pronggroup y game_handler a pongclient)
	group_nelh->add_event_listener("game/event", [] (boost::json::object& data) {

		controller.push_event(data);
		
	});
	
}