#ifndef server_hpp
#define server_hpp

#include <boost/asio.hpp>

#include "clients.h"

class Server {

	uint16_t port;

	boost::asio::io_context io_context;

	boost::asio::ip::tcp::acceptor acceptor;

	boost::asio::ip::tcp::endpoint tcp_local_endpoint;

	boost::asio::ip::udp::endpoint udp_local_endpoint;
	
	UdpController udp_controller;

	Client* clients[10];

	uint8_t verbose = 0;

	const uint16_t max_connections = 3;

	void wait_for_connection();

	void on_new_connection(boost::asio::ip::tcp::socket& socket);
	
	void remove_dead_connections();

	void remove_client(uint16_t idx);
	
	void start_listening();

public:

	Server(uint16_t _port);

	void run();

};
#endif /* server_hpp */
