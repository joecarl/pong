#include "group.h"

using namespace std;


boost::json::object export_player(PlayerP* p) {

	boost::json::object o;

	o["x"] = p->x;
	o["y"] = p->y;
	o["com_txt_y"] = p->com_txt_y;
	o["score"] = p->score;
	o["medlen"] = p->medlen;
	o["racha"] = p->racha;

	boost::json::array arr1;
	for (auto &v: p->bonus_timers) {
		arr1.push_back(v);
	}
	o["bonus_timers"] = arr1;

	boost::json::array arr2;
	for (auto &v: p->controls) {
		arr2.push_back(v);
	}
	o["controls"] = arr2;
	
	return o;

}

boost::json::object export_element(Element* e) {

	boost::json::object o;

	o = {
		{"stat", e->stat},
		{"x", e->x},
		{"y", e->y},
		{"x00", e->x00},
		{"y00", e->y00},
		{"radius", e->radius},
		{"vx", e->vx},
		{"vy", e->vy},
		{"t", e->t}
	};

	return o;

}

boost::json::object export_ball(Ball* b) {
	
	return export_element((Element*) b);
	
}

boost::json::object export_bonus(Bonus* b) {

	boost::json::object o = export_element((Element*) b);
	o["cooldown"] = b->cooldown;
	
	return o;

}


boost::json::object export_game(PongGame* g) {

	boost::json::object o;

	o = {
		{"tick", g->tick},
		{"warmup", g->warmup},
		{"paused", g->paused},
		{"rnd_index", g->rnd.index},
		{"p0vars", export_player(g->players[0])},
		{"p1vars", export_player(g->players[1])},
		{"bonus0vars", export_bonus(g->bonus[0])},
		{"bonus1vars", export_bonus(g->bonus[1])},
		{"ballvars", export_ball(g->ball)}
	};

	return o;

}



Group::Group() {

	//this->new_game();

}

void Group::new_game() {

	delete this->game;
	
	//vaciamos la cola de eventos
	queue<boost::json::object> empty;
	swap(this->evt_queue, empty);

	this->game = new PongGame();

	this->game->control_mode = CONTROLMODE_TWO_PLAYERS;

	this->game->restart();

	this->game->iniciar_punto(1);

	//cout << "New game created, tick: " << this->game->tick << endl;

	this->players_ready[0] = false;
	this->players_ready[1] = false;

}

void Group::add_client(Client* cl) {

	int player_id;

	bool must_push = true;

	for (size_t i = 0; i < this->clients.size(); i++) {//} &itCl: this->clients) {
		if (this->clients[i] == nullptr) {
			this->clients[i] = cl;
			must_push = false;
			player_id = i;
			break;
		}
	}

	if (must_push) {
		player_id = this->clients.size();
		this->clients.push_back(cl);
	}

	cl->add_event_listener("onDrop", [this, player_id, cl] () {

		//cout << "onDrop event callback!!" << endl;

		if (this->clients[player_id] == cl) {
			this->clients[player_id] = nullptr;
			cout << "Client #" << player_id << " dropped from group" << endl;
		}
		
	});

	if (player_id < 2) {
		
		cl->on_pkg_received = [this, player_id] (boost::json::object& pkg) {

			auto evtType = pkg["type"].as_string();
		
			if (evtType == "ready_to_play") {//if scope == group-event

				cout << "Player ready!!" << endl;
				
				this->players_ready[player_id] = true;

				if (this->players_ready[0] && this->players_ready[1]) {
					this->start_game();
				}

			} else if (evtType == "desync") {

				boost::json::object o = {
					{"type", "sync"},
					{"gamevars", export_game(this->game)}
				};

				cerr << "!! RESYNC " << o << endl;

				this->clients[player_id]->qsend_udp(boost::json::serialize(o));

			} else {

			//if (evtType == "set_control_state") {//if scope == game-event
				cout << pkg << endl;

				unsigned int origTick = pkg["tick"].as_int64();

				unsigned int tickDiff = 0;

				if (origTick < this->game->tick) {
					tickDiff = this->game->tick - origTick;
					cout << "Tick diff: " << tickDiff << endl;
				}

				pkg["player_key"] = player_id;
				pkg["tick"] = this->game->tick + 0; //TODO: auto calc tick delay based on clients connection?

				this->evt_queue.push(pkg);

				this->send_to_all(boost::json::serialize(pkg));
			//}
			}
		
		};

	}

}

void Group::process_event(boost::json::object &evt) {

	//cout << "processing EVT: " << evt << endl;

	auto evtType = evt["type"].as_string();
		
	if (evtType == "set_control_state") {
		
		int control = evt["control"].as_int64();
		bool newState = evt["state"].as_bool();
		int player_id = evt["player_key"].as_int64();
		
		this->game->players[player_id]->controls[control] = newState;

	} else {
		cerr << "Unknown event type: " << evtType << endl;
	}

}

void Group::start_game() {

	cout << "Starting game in group" << endl;

	this->new_game();

	io = new boost::asio::io_context();

	t = new boost::asio::steady_timer(*io, boost::asio::chrono::milliseconds(75 * 1000 / 60)); //el 75 se corresponde al delayer, quiza deberia commonizarse

	t->async_wait(boost::bind(&Group::game_main_loop, this));

	boost::json::object pkg = {
		{"type", "game_start"},
		{"seed", this->game->rnd.get_seed()}
	};

	this->send_to_all(boost::json::serialize(pkg));

	boost::thread(boost::bind(&boost::asio::io_context::run, io));

}

void Group::send_to_all(const string& pkg) {

	for (auto& cl : this->clients) {
		if (cl != nullptr && !cl->is_dead()) {
			//if (cl->logged || 1) {
				cl->qsend_udp(pkg);
			//}
		}
	}

}


void Group::game_main_loop() {

	//cout << "game_main_loop" << endl;

	while (this->evt_queue.size() > 0) {

		auto evt = this->evt_queue.front();

		//cout << "checking EVT: " << evt << " | is uint64 ? " << evt["tick"].is_uint64() << endl;

		unsigned int evtTick = (unsigned int)(evt["tick"].as_uint64());

		if (evtTick == this->game->tick) {

			this->process_event(evt);

			this->evt_queue.pop();

		} else if (evtTick < this->game->tick) {

			throw runtime_error("Evento perdido");

		} else {
			
			break;

		}

	}

	this->game->process_tick();

	if (this->game->finished) {
		
		//this->new_game();

		return;

	}

	useconds_t usec = 1000000 / 60;
	
	t->expires_at(t->expiry() + boost::asio::chrono::microseconds(usec));

	t->async_wait(boost::bind(&Group::game_main_loop, this));

}