//
//  ioclient.h
//  pong
//
//  Created by Joe on 13/9/18.
//

#ifndef IOCLIENT_H
#define IOCLIENT_H

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/json.hpp>

#include <chrono>
#include <queue>

#include "../udpcontroller.h"

enum ConnState {
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

/**
 * Handles the connection with the server. Enables TCP and UDP communication
 */
class IoClient {

	int pkgs_sent = 0, pkgs_recv = 0;

	std::queue<qsend_item> pkg_queue;
	
	std::vector<callb> cbs;

	boost::asio::io_context io_context;
	
	boost::asio::ip::tcp::socket socket;

	UdpController* udp_controller;

	UdpChannelController* udp_channel;

	bool busy = false;

	ConnState connection_state = CONNECTION_STATE_DISCONNECTED;

	int id_client = 0;

	unsigned int pkg_id = 0;

	unsigned char read_buffer[1024];

	std::string read_remainder = "";

	bool wait_for_binary = false;

	boost::json::object wait_for_binary_pt;

	void send_app_info();

	void setup_udp(std::string& local_id);

	void start_ping_thread();

	void handle_qsent_content(const boost::system::error_code& error, std::size_t bytes_transferred);

	void handle_qread_content(const boost::system::error_code& error, std::size_t bytes_transferred);

	void save_cb(const std::string& pkg, const callback_fn_type& _cb);
	
	void qread();

	callback_fn_type process_actions_fn;

	int64_t ping_ms = 0;

	std::string current_host;

public:

	IoClient();

	~IoClient();

	/**
	 * Connects to the specified address and port, the function always returns
	 * immediately and the state will change to `CONNECTION_STATE_CONNECTING`.
	 * If the process is successful the state will eventually change to 
	 * `CONNECTION_STATE_CONNECTED`. At this point TCP communication is 
	 * available and calling `qsend` should work. After this, a local id will
	 * be received and stored, a UDP channel will be created and when it is 
	 * fully established the state will change to 
	 * `CONNECTION_STATE_CONNECTED_FULL`.
	 */
	void connect(const std::string& addr, unsigned short port);

	/**
	 * Obtains the connection state.
	 */
	ConnState get_state();

	/**
	 * Sends data via TCP socket.
	 */
	void qsend(std::string pkg, const callback_fn_type& _cb = nullptr);

	/**
	 * Sends data via UDP socket.
	 */
	void qsend_udp(const std::string& pkg, const callback_fn_type& _cb = nullptr);

	/**
	 * Retrieves the local id provided by the server. This function should not
	 * be called before the connection state reaches 
	 * `CONNECTION_STATE_CONNECTED_FULL`.
	 */
	const std::string& get_local_id();

	/**
	 * Sets the application level callback used to process custom packages.
	 */
	void set_process_actions_fn(const callback_fn_type& _fn);

	/**
	 * Obtains the last host that the client connected to.
	 */
	std::string& get_current_host() { return this->current_host; }

	/**
	 * Obtains the current ping in milliseconds.
	 */
	int64_t get_ping_ms() { return this->ping_ms; }

};


#endif
