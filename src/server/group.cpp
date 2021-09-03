#include "group.h"

using namespace std;

Group::Group(){

	this->newGame();

}

void Group::newGame(){

	if(this->game != nullptr){
		delete this->game;
	}

	this->game = new PongGame();

	cout << "New game created, tick: " << this->game->tick << endl;

	this->players_ready = 0;

}

void Group::addClient(Client* cl){

	int playerID = this->clients.size();

	this->clients.push_back(cl);

	if(playerID < 2 ){
		
		cl->on_pkg_received = [this, playerID](boost::json::object& pkg){

			auto evtType = pkg["type"].as_string();
		
			if(evtType == "play_again"){//if scope == group-event
				cout << "Player ready!!" << endl;
				this->players_ready ++;
				if(this->players_ready == 2){
					this->startGame();
				}
			} else {

			//if(evtType == "set_control_state"){//if scope == game-event
				cout << pkg << endl;

				pkg["playerKey"] = playerID;
				pkg["tick"] = this->game->tick + 4;

				this->evt_queue.push(pkg);

				this->sendToAll(boost::json::serialize(pkg));
			//}
			}
		
		};

	}

	auto numClients = this->clients.size();

	if(numClients == 2){
		
		this->startGame();

	}

}

void Group::process_event(boost::json::object &evt){

	//cout << "processing EVT: " << evt << endl;

	auto evtType = evt["type"].as_string();
		
	if(evtType == "set_control_state"){
		
		int control = evt["control"].as_int64();
		bool newState = evt["state"].as_bool();
		int playerID = evt["playerKey"].as_int64();
		
		this->game->players[playerID]->controls[control] = newState;

	} else {
		cerr << "Unknown event type: " << evtType << endl;
	}

}

void Group::startGame(){

	cout << "Starting game in group" << endl;

	t = new boost::asio::steady_timer(io, boost::asio::chrono::milliseconds(75 * 1000 / 60)); //el 75 se corresponde al delayer, quiza deberia commonizarse

	t->async_wait(boost::bind(&Group::game_main_loop, this));

	boost::json::object pkg = {
		{"type", "game_start"},
		{"seed", this->game->seed}
	};

	this->sendToAll(boost::json::serialize(pkg));

	//io.run();
	boost::thread(boost::bind(&boost::asio::io_context::run, &io));

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


void Group::game_main_loop(){

	//cout << "game_main_loop" << endl;

	while(this->evt_queue.size() > 0){

		auto evt = this->evt_queue.front();

		//cout << "checking EVT: " << evt << " | is uint64 ? " << evt["tick"].is_uint64() << endl;

		unsigned int evtTick = (unsigned int)(evt["tick"].as_uint64());

		if(evtTick == this->game->tick){

			this->process_event(evt);

			this->evt_queue.pop();

		} else if (evtTick < this->game->tick){

			throw std::runtime_error("Evento perdido");

		} else {
			
			break;

		}

	}

	this->game->processTick();

	if(this->game->finished){
		
		this->newGame();

		return;

	}

	useconds_t usec = 1000000 / 60;
	
	t->expires_at(t->expiry() + boost::asio::chrono::microseconds(usec));

	t->async_wait(boost::bind(&Group::game_main_loop, this));

}