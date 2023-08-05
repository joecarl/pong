#include "server.h"
#include "group.h"

#include <ctime>
#include <iostream>
#include <stdlib.h>

#include <boost/json.hpp>
#include <boost/chrono.hpp>

using namespace boost::asio::ip;
using namespace std;

Group group;

Server::Server(int _port): io_context(), acceptor(io_context) {

	this->port = _port;
	this->endpoint = tcp::endpoint(tcp::v4(), port);

	this->start_listening();

	udp::endpoint local_endpoint(udp::v4(), _port);
	
	this->udp_controller = new UdpController(io_context.get_executor(), local_endpoint, "SERVER");
	this->udp_controller->on_new_channel = [this] (UdpChannelController& ch) {
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


void Server::remove_client(int idx) {

	delete clients[idx];

	clients[idx] = nullptr;

}

void Server::start_listening() {

	boost::system::error_code ec;

	if (!this->acceptor.is_open()) {
		cout << "Opening acceptor " << this->endpoint << endl;

		this->acceptor.open(this->endpoint.protocol());
		
		this->acceptor.set_option(tcp::acceptor::reuse_address(true));

		this->acceptor.bind(this->endpoint);
		
		this->acceptor.listen();
	}
}


void Server::on_new_connection(tcp::socket& socket) {

	for (int i = 0; i < this->max_connections; i++) {

		cout << "Testing " << i << ": " << (clients[i] == nullptr ? "available!" : "busy") << endl;

		if (clients[i] == nullptr) {

			auto cl = new Client(socket);

			clients[i] = cl;
			
			std::cout << "Client " << i << " connected!" << std::endl;

			group.add_client(cl);
			
			cl->async_wait_for_data();

			break;

		} 

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
