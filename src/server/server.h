#ifndef server_hpp
#define server_hpp

#include <stdio.h>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "clients.h"

class Server{

	int port;

	int verbose = 0;

	Client* clients[10];

	const int max_connections = 3;
	
	boost::asio::ip::tcp::endpoint endpoint;

	void wait_for_connection();

	void on_new_connection(boost::asio::ip::tcp::socket socket);
	
	void remove_dead_connections();

	void remove_client(int idx);
	
	//bool is_full();
	
	void start_listening();
	
	//void stop_listening();

public:

	boost::asio::io_context io_context;
	boost::asio::ip::tcp::acceptor acceptor;

	Server(int _port);

	void run();

};
#endif /* server_hpp */
