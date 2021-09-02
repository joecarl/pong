#ifndef server_hpp
#define server_hpp

#include <stdio.h>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "clients.h"

class Server{

	int port = 25000;

	int timeout = 0;

	int interactions = -1;

	int ret = 1;

	int verbose = 0;

	char* opt;

	Client* clients[10];

	const int max_connections = 3;

    void poll();
    
	boost::asio::ip::tcp::endpoint endpoint;
	
	void game_main_loop(boost::asio::steady_timer* t);
	
public:
	//std::function<void(boost::property_tree::ptree& pt, Client* cl)> process_actions_fn;
	
	boost::asio::io_service io_service;
	boost::asio::ip::tcp::acceptor acceptor;

	Server(int _port);

	void run();
	
	bool is_full();
	
	void kick_client(int idx);
	
	void start_listening();
	
	void stop_listening();

	//void process_clients_requests();

	//void send_to_all(std::string pkg);

};
#endif /* server_hpp */
