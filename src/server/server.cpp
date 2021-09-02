#include "server.h"
#include "group.h"

#include <ctime>
#include <iostream>
#include <stdlib.h>

#include <boost/json.hpp>
#include <boost/chrono.hpp>

using namespace boost::asio::ip;
using namespace std;


Server::Server(int _port): io_service(), acceptor(io_service){

	this->port = _port;
	this->endpoint = tcp::endpoint(tcp::v4(), port);

	this->start_listening();

	for(int i = 0; i < this->max_connections; i++){
		clients[i] = nullptr;
	}

}

/*
void Server::process_clients_requests(){

	for(int i = 0; i < this->max_connections; i++){
		if(clients[i] == nullptr) continue;
	}
	
}
*/

void Server::poll(){

	while(1){

		this->io_service.restart();//en windows no hacia falta hacer restart :/
		this->io_service.run();
		boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
		//cout << "polling!"<< endl;

	}

}

bool Server::is_full(){

	for(int i = 0; i < this->max_connections; i++){
		if(clients[i] == nullptr){
			return false;
		}
		else if(clients[i]->is_dead() || !clients[i]->is_connected()){
			return false;
		}
	}
	return true;
}


void Server::kick_client(int idx){

	delete clients[idx];

	clients[idx] = nullptr;

}

void Server::start_listening(){

	boost::system::error_code ec;

	if(!this->acceptor.is_open()){
		cout << "Opening acceptor " << this->endpoint << endl;

		this->acceptor.open(this->endpoint.protocol(), ec);
		//cout << ec << endl;
		this->acceptor.set_option(tcp::acceptor::reuse_address(true));
		this->acceptor.bind(this->endpoint, ec);
		//cout << ec << endl;
		this->acceptor.listen();
	}
}


void Server::stop_listening(){

	boost::system::error_code ec;

	if(this->acceptor.is_open()){
		cout << "Stopping acceptor" << endl;
		this->acceptor.cancel(ec);
		cout << ec << endl;
		this->acceptor.close(ec);
		cout << ec << endl;

	}
}

/*
void Server::send_to_all(std::string pkg){
	for(int i = 0; i < this->max_connections; i++){
		if(clients[i] != nullptr && !clients[i]->is_dead()){
			if(clients[i]->logged || 1){
				clients[i]->qsend(pkg);
			}
		}
	}
}
*/

void Server::run(){

	Group group;

	boost::asio::io_context io;

	//io.run();
	boost::thread th0(boost::bind(&boost::asio::io_context::run, &io));

	//boost::system::error_code ec;
	boost::thread th(boost::bind(&Server::poll, this));

	while(1){

		if(this->is_full()){

			this->stop_listening();

		} else {
			//cout << "ep acceptor " << this->acceptor.local_endpoint() << endl;
			if(!this->acceptor.is_open()){

				this->start_listening();

			}

			for(int i = 0; i < this->max_connections; i++){
				cout << "Testing " << i << ": " << (clients[i] == nullptr ? "available!" : "busy") << endl;
				if(clients[i] == nullptr){
					//sockets[i] = new

					cout << "Waiting for connection..." << endl;

					clients[i] = new Client(io_service, acceptor);
					if(!clients[i]->conn_err){

						std::cout << "Client " << i << " connected!" << std::endl;

						group.addClient(clients[i]);
						
						clients[i]->async_wait_for_data();

					}

				} else if(clients[i]->is_dead()) {
					//get rid of dead connections
					this->kick_client(i);
				}

			}
		}


		boost::this_thread::sleep_for(boost::chrono::milliseconds(100));

	}

}
