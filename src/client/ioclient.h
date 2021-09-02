//
//  connections.hpp
//  otherside
//
//  Created by Joe on 13/9/18.
//

#ifndef connections_hpp
#define connections_hpp
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/json.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <chrono>
#include <queue>


struct callb{
	std::string name;
	std::function<void(boost::json::object& pt)> cb;
	std::chrono::time_point<std::chrono::high_resolution_clock> c_s;
};

struct qsend_item{
	std::string pkg;
	std::function<void(boost::json::object& pt)> _cb;
};

class client{
	int pkgs_sent = 0, pkgs_recv = 0;

	std::queue<qsend_item> pkg_queue;
	std::vector<callb> cbs;

	boost::asio::io_service io_service;
	//boost::asio::ip::tcp::resolver resolver;
	//boost::asio::ip::tcp::resolver::query query;
	//boost::asio::ip::tcp::resolver::iterator endpoint_iterator;
	boost::asio::ip::tcp::endpoint endpoint;
	boost::asio::ip::tcp::socket socket;
	bool dead = false, busy = false;
	int id_client = 0;
	unsigned int pkg_id = 0;
	unsigned char read_buffer[1024];
	std::string read_remainder = "";
	bool wait_for_binary = false;
	boost::json::object wait_for_binary_pt;

	void handle_qsent_content(const boost::system::error_code& error, std::size_t bytes_transferred);
	void handle_sent_content(const boost::system::error_code& error, std::size_t bytes_transferred);
	void handle_read_content(const boost::system::error_code& error, std::size_t bytes_transferred);
	void handle_qread_content(const boost::system::error_code& error, std::size_t bytes_transferred);

public:
	float ping_ms = 0;
	client(std::string addr, unsigned short port);
	~client();
	bool is_alive();
	void send(std::string pkg, std::function<void(std::string& res)> _cb);
	void qsend(std::string pkg, std::function<void(boost::json::object& pt)> _cb = nullptr);
	void qread();
	std::function<void(boost::json::object& pt)> process_actions_fn;

};

void read_socket_task(boost::asio::ip::tcp::socket* socket);
void connection_task();
#endif /* connections_hpp */
