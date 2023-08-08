//
//  ioclient.hpp
//  pong
//
//  Created by Joe on 13/9/18.
//

#ifndef IOCLIENT_HPP
#define IOCLIENT_HPP

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/json.hpp>

#include <chrono>
#include <queue>

#include "../udpcontroller.h"

enum {
	CONNECTION_STATE_DISCONNECTED = 0,
	CONNECTION_STATE_CONNECTING,
	CONNECTION_STATE_CONNECTED,
	CONNECTION_STATE_CONNECTED_FULL
};

typedef std::function<void(boost::json::object& pt)> callback_fn_type;

struct callb {
	std::string name;
	callback_fn_type cb;
	std::chrono::time_point<std::chrono::high_resolution_clock> c_s;
};

struct qsend_item {
	std::string pkg;
	callback_fn_type _cb;
};

class UdpController;

class IoClient {

	int pkgs_sent = 0, pkgs_recv = 0;

	std::queue<qsend_item> pkg_queue;
	
	std::vector<callb> cbs;

	boost::asio::io_context io_context;
	
	boost::asio::ip::tcp::socket socket;

	UdpController* udp_controller;

	UdpChannelController* udp_channel;

	bool busy = false;

	int connection_state = CONNECTION_STATE_DISCONNECTED;

	int id_client = 0;

	unsigned int pkg_id = 0;

	unsigned char read_buffer[1024];

	std::string read_remainder = "";

	bool wait_for_binary = false;

	boost::json::object wait_for_binary_pt;

	void setup_udp(std::string& local_id);

	void start_ping_thread();

	void handle_qsent_content(const boost::system::error_code& error, std::size_t bytes_transferred);
	//void handle_sent_content(const boost::system::error_code& error, std::size_t bytes_transferred);
	//void handle_read_content(const boost::system::error_code& error, std::size_t bytes_transferred);
	void handle_qread_content(const boost::system::error_code& error, std::size_t bytes_transferred);
	void save_cb(const std::string& pkg, const callback_fn_type& _cb);

	callback_fn_type process_actions_fn;

	int64_t ping_ms = 0;

	std::string current_host;

public:

	IoClient();

	~IoClient();

	void connect(const std::string& addr, unsigned short port);

	int get_state();

	void qsend(std::string pkg, const callback_fn_type& _cb = nullptr);

	void qsend_udp(const std::string& pkg, const callback_fn_type& _cb = nullptr);

	void qread();

	void set_process_actions_fn(const callback_fn_type& _fn);

	std::string& get_current_host() { return this->current_host; }

	int64_t get_ping_ms() { return this->ping_ms; }

};


#endif
