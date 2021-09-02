//
//  server.cpp
//  mysql_test
//
//  Created by Joe on 11/9/18.
//  Copyright © 2018 Joe. All rights reserved.
//

#include <ctime>

#include <iostream>
#include <stdlib.h>
#include "server.h"

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

	//la inicializacion se hace una vez global para cada servidor
	//world::init();

}

void Server::process_clients_requests(){

	for(int i = 0; i < this->max_connections; i++){
		if(clients[i] == nullptr) continue;
	}
    
}


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

void Server::send_to_all(std::string pkg){
	for(int i = 0; i < this->max_connections; i++){
		if(clients[i] != nullptr && !clients[i]->is_dead()){
			if(clients[i]->logged || 1){
				clients[i]->qsend(pkg);
			}
		}
	}
}

void Server::game_main_loop(boost::asio::steady_timer* t){

	useconds_t usec = 1000000 / 60;
	//static int counter = 0;

	for(int i = 0; i < this->max_connections; i++){
		if(clients[i] != nullptr && !clients[i]->is_dead()){
			//if(clients[i]->logged || 1){
/*
				clients[i]->pl.process_keyb_move(clients[i]->keyb);
				clients[i]->pl.process_physics();//optimize this function!

				Client* mcl = clients[i];

				string vars =
				to_string(mcl->pl.x) + "," +
				to_string(mcl->pl.y) + "," +
				(mcl->keyb[GLFW_KEY_W] ? "1" : "0") + "," +
				string pkg = "{\"action\": \"update_pl\", \"vars\": \"" + vars + "\"}";
				mcl->qsend(pkg);
*/
			//}

		}
	}
    /*
	if(counter ++ > 60){
		counter = 0;
		//cout << "S" << endl;

		for(int i = 0; i < this->max_connections; i++){
			if(clients[i] != nullptr && !clients[i]->is_dead()){
				cout << "[C" << i << "]" << "X: " << clients[i]->pl.x << " | Z: " << clients[i]->pl.z << endl;
			}
		}
	}
*/
	t->expires_at(t->expiry() + boost::asio::chrono::microseconds(usec));
	t->async_wait(boost::bind(&Server::game_main_loop, this, t));

}

void Server::run(){

	useconds_t usec = 1000000 / 60;

	boost::asio::io_context io;

	boost::asio::steady_timer t(io, boost::asio::chrono::microseconds(usec));

	t.async_wait(boost::bind(&Server::game_main_loop, this, &t));

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
