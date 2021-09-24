#include "group.h"

using namespace std;


boost::json::object exportPlayer(PlayerP* p){

	boost::json::object o;

	o["x"] = p->x;
	o["y"] = p->y;
	o["comTxtY"] = p->comTxtY;
	o["score"] = p->score;
	o["medlen"] = p->medlen;
	o["racha"] = p->racha;

	boost::json::array arr1;
	for(auto &v: p->bonus_timers){
		arr1.push_back(v);
	}
	o["bonus_timers"] = arr1;

	boost::json::array arr2;
	for(auto &v: p->controls){
		arr2.push_back(v);
	}
	o["controls"] = arr2;
	
	return o;

}

boost::json::object exportElement(Element* e){

	boost::json::object o;

	o = {
		{"stat", e->stat},
		{"x", e->x},
		{"y", e->y},
		{"x00", e->x00},
		{"y00", e->y00},
		{"radius", e->radius},
		{"vX", e->vX},
		{"vY", e->vY},
		{"t", e->t}
	};

	return o;

}

boost::json::object exportBall(Ball* b){
	
	return exportElement((Element*) b);
	
}

boost::json::object exportBonus(Bonus* b){

	boost::json::object o = exportElement((Element*) b);
	o["cooldown"] = b->cooldown;
	
	return o;

}


boost::json::object exportGame(PongGame* g){

	boost::json::object o;

	o = {
		{"tick", g->tick},
		{"paused", g->paused},
		{"p0vars", exportPlayer(g->players[0])},
		{"p1vars", exportPlayer(g->players[1])},
		{"bonus0vars", exportBonus(g->bonus[0])},
		{"bonus1vars", exportBonus(g->bonus[1])},
		{"ballvars", exportBall(g->ball)}
	};

	return o;

}



Group::Group(){

	this->newGame();

}

void Group::newGame(){

	if(this->game != nullptr){
		delete this->game;
	}

	//vaciamos la cola de eventos
	std::queue<boost::json::object> empty;
	std::swap( this->evt_queue, empty );

	this->game = new PongGame();

	this->game->controlMode = CONTROLMODE_TWO_PLAYERS;

	this->game->restart();

	this->game->iniciarPunto(1);

	//cout << "New game created, tick: " << this->game->tick << endl;

	this->players_ready[0] = false;
	this->players_ready[1] = false;

}

void Group::addClient(Client* cl){

	int playerID;

	bool mustPush = true;

	for(size_t i = 0; i < this->clients.size(); i++){//} &itCl: this->clients){
		if (this->clients[i] == nullptr){
			this->clients[i] = cl;
			mustPush = false;
			playerID = i;
			break;
		}
	}

	if(mustPush){
		playerID = this->clients.size();
		this->clients.push_back(cl);
	}

	cl->addEventListener("onDrop", [this, playerID, cl](){

		//cout << "onDrop event callback!!" << endl;

		if (this->clients[playerID] == cl ){
			this->clients[playerID] = nullptr;
			cout << "Client #" << playerID << " dropped from group" << endl;
		}
		
	});

	if(playerID < 2 ){
		
		cl->on_pkg_received = [this, playerID](boost::json::object& pkg){

			auto evtType = pkg["type"].as_string();
		
			if(evtType == "ready_to_play"){//if scope == group-event

				cout << "Player ready!!" << endl;
				
				this->players_ready[playerID] = true;

				if(this->players_ready[0] && this->players_ready[1]){
					this->startGame();
				}

			} else if(evtType == "desync"){

				boost::json::object o = {
					{"type", "sync"},
					{"gamevars", exportGame(this->game)}
				};

				this->clients[playerID]->qsend(boost::json::serialize(o));

			} else {

			//if(evtType == "set_control_state"){//if scope == game-event
				cout << pkg << endl;

				unsigned int origTick = pkg["tick"].as_int64();

				unsigned int tickDiff = 0;

				if(origTick < this->game->tick){
					tickDiff = this->game->tick - origTick;
					cout << "Tick diff: " << tickDiff << endl;
				}

				pkg["playerKey"] = playerID;
				pkg["tick"] = this->game->tick + 3; //TODO: auto calc tick delay based on clients connection?

				this->evt_queue.push(pkg);

				this->sendToAll(boost::json::serialize(pkg));
			//}
			}
		
		};

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

	io = new boost::asio::io_context();

	t = new boost::asio::steady_timer(*io, boost::asio::chrono::milliseconds(75 * 1000 / 60)); //el 75 se corresponde al delayer, quiza deberia commonizarse

	t->async_wait(boost::bind(&Group::game_main_loop, this));

	boost::json::object pkg = {
		{"type", "game_start"},
		{"seed", this->game->seed}
	};

	this->sendToAll(boost::json::serialize(pkg));

	boost::thread(boost::bind(&boost::asio::io_context::run, io));

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