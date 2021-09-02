//
//  clients.hpp
//  mysql_test
//
//  Created by Joe on 10/9/18.
//  Copyright © 2018 Joe. All rights reserved.
//

#ifndef clients_hpp
#define clients_hpp

#include <boost/json.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <queue>

class Client{
	//game vars
	int pkgs_recv = 0;
	int pkgs_sent = 0;
	//conn vars
	std::queue<std::string> pkg_queue;
	std::string pending_data = "";
	bool busy = false;

	static int count_instances;
    int id_client = 0;
	bool dead = false;
	bool connected = false;
	boost::asio::ip::tcp::socket socket;

	unsigned char read_buffer[1024];

	std::string read_remainder = "";

	void handle_read_content( const boost::system::error_code& error, std::size_t bytes_transferred);
	void handle_qsent_content( const boost::system::error_code& error, std::size_t bytes_transferred);

	void process_stage_action(boost::json::object& pt);

public:
	
	int stage = 0;
	//std::queue<boost::property_tree::ptree> actions;


	//game vars
    bool keyb[1000];//GLFW_KEY_MAX?
	bool mouse[10];

	//player pl;
	bool logged = false;

	//conn vars
	boost::system::error_code conn_err;
	Client(boost::asio::io_service& io_service, boost::asio::ip::tcp::acceptor& acceptor);
	~Client();

	void async_wait_for_data();
	void qsend(std::string pkg, bool ignore_busy = false);
	bool is_dead();
	bool is_connected();

	void process_request(std::string request);

};

#endif
