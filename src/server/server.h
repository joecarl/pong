#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>

#include "clients.h"

class Server {

	/**
	 * The port where the server will listen (used for TCP and UDP).
	 */
	uint16_t port;

	/**
	 * The execution context.
	 */
	boost::asio::io_context io_context;

	/**
	 * The acceptor which will handle incoming TCP connections.
	 */
	boost::asio::ip::tcp::acceptor acceptor;

	/**
	 * The local endpoint used for tcp connections.
	 */
	boost::asio::ip::tcp::endpoint tcp_local_endpoint;

	/**
	 * The local endpoint where the udp socket will receive data.
	 * When comparing tcp and udp endpoints as strings there should be no 
	 * difference.
	 */
	boost::asio::ip::udp::endpoint udp_local_endpoint;
	
	/**
	 * The controller which will manage udp datagrams
	 */
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

#endif
