#include "server.h"
#include "group.h"
#include "../utils.h"

#include <ctime>
#include <iostream>
#include <stdlib.h>

#include <boost/json.hpp>
#include <boost/chrono.hpp>

using namespace boost::asio::ip;
using namespace std;

Group group;

Server::Server(uint16_t _port): 
	port(_port),
	io_context(), 
	acceptor(io_context),
	tcp_local_endpoint(tcp::v4(), port),
	udp_local_endpoint(udp::v4(), port),
	udp_controller(io_context.get_executor(), udp_local_endpoint, "SERVER")
{

	this->start_listening();

	this->udp_controller.on_new_channel = [this] (UdpChannelController& ch) {
		for (int i = 0; i < this->max_connections; i++) {
			auto cl = clients[i];
			if (cl == nullptr) {
				continue;
			}
			auto cl_id = "C" + to_string(cl->get_id());
			if (cl_id == ch.get_remote_id()) {
				cl->set_udp_channel(ch);
			}
		}
	};

	for (int i = 0; i < this->max_connections; i++) {
		clients[i] = nullptr;
	}

}


void Server::remove_client(uint16_t idx) {

	delete clients[idx];

	clients[idx] = nullptr;

}

void Server::start_listening() {

	boost::system::error_code ec;

	if (!this->acceptor.is_open()) {
		cout << "Opening acceptor " << this->tcp_local_endpoint << endl;

		this->acceptor.open(this->tcp_local_endpoint.protocol());
		
		this->acceptor.set_option(tcp::acceptor::reuse_address(true));

		this->acceptor.bind(this->tcp_local_endpoint);
		
		this->acceptor.listen();
	}
}


void Server::on_new_connection(tcp::socket& socket) {

	cout << "[" << date() << "] New connection!" << endl;
	bool assigned = false;

	for (int i = 0; i < this->max_connections; i++) {

		//cout << "Testing " << i << ": " << (clients[i] == nullptr ? "available!" : "busy") << endl;

		if (clients[i] == nullptr) {

			auto cl = new Client(socket);

			clients[i] = cl;
			
			std::cout << "Client " << i << " connected!" << std::endl;

			group.add_client(cl);
			
			cl->async_wait_for_data();

			assigned = true;

			break;

		} 

	}

	if (!assigned) {
		cerr << "Unable to find an available slot for the client" << endl;
	}

}


void Server::remove_dead_connections() {

	for (int i = 0; i < this->max_connections; i++) {

		if (clients[i] != nullptr && clients[i]->is_dead()) {
			//get rid of dead connections
			this->remove_client(i);
		}
	}

}


void Server::wait_for_connection() {

	this->remove_dead_connections();

	cout << "Waiting for connection..." << endl;

	acceptor.async_accept([this] (boost::system::error_code ec, tcp::socket socket) {

		if (ec) {
			cout << "Error opening socket: " << ec << endl;
		} else {
			this->on_new_connection(socket);
		}

		this->wait_for_connection();

	});
		
}


void Server::run() {
	
	this->wait_for_connection();
	
	this->io_context.run();

}
