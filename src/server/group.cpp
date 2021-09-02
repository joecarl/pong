#include "group.h"

using namespace std;

Group::Group(){

}

void Group::addClient(Client* cl){

	int playerID = this->clients.size();

	this->clients.push_back(cl);

	if(playerID < 2 ){
		
		cl->on_pkg_received = [=](boost::json::object& pkg){

			auto evtType = pkg["type"].as_string();
		
			if(evtType == "set_control_state"){
				cout << pkg << endl;

				int control = pkg["control"].as_int64();
				bool newState = pkg["state"].as_bool();
				//int playerKey = pkg["playerKey"].as_int64();
				pkg["playerKey"] = playerID;
				this->game.players[playerID]->controls[control] = newState;

				
				this->sendToAll(boost::json::serialize(pkg));
			}
		
		};

	}

	auto numClients = this->clients.size();

	if(numClients == 2){
		
		this->startGame();

	}

}

void Group::startGame(){

	cout << "Starting game in group" << endl;

	boost::asio::io_context io;

	boost::asio::steady_timer t(io, boost::asio::chrono::milliseconds(1));

	t.async_wait(boost::bind(&Group::game_main_loop, this, &t));

	boost::json::object pkg = {
		{"type", "game_start"},
		{"seed", this->game.seed}
	};

	this->sendToAll(boost::json::serialize(pkg));

}

void Group::sendToAll(std::string pkg){

	for(auto& cl : this->clients){
		if(cl != nullptr && !cl->is_dead()){
			//if(cl->logged || 1){
				cl->qsend(pkg);
			//}
		}
	}

}


void Group::game_main_loop(boost::asio::steady_timer* t){

	useconds_t usec = 1000000 / 60;

	this->game.processTick();
	
	t->expires_at(t->expiry() + boost::asio::chrono::microseconds(usec));
	t->async_wait(boost::bind(&Group::game_main_loop, this, t));

}