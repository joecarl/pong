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

#include <chrono>
#include <queue>

enum {
	CONNECTION_STATE_DISCONNECTED = 0,
	CONNECTION_STATE_CONNECTING,
	CONNECTION_STATE_CONNECTED
};


struct callb {
	std::string name;
	std::function<void(boost::json::object& pt)> cb;
	std::chrono::time_point<std::chrono::high_resolution_clock> c_s;
};

struct qsend_item {
	std::string pkg;
	std::function<void(boost::json::object& pt)> _cb;
};

class IoClient {

	int pkgs_sent = 0, pkgs_recv = 0;

	std::queue<qsend_item> pkg_queue;
	std::vector<callb> cbs;

	boost::asio::io_context io_context;
	
	boost::asio::ip::tcp::socket socket;

	bool busy = false;

	int connection_state = CONNECTION_STATE_DISCONNECTED;

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

	std::string current_host;

	int64_t ping_ms = 0;

	IoClient();

	~IoClient();

	void connect(const std::string& addr, unsigned short port);

	int get_state();

	//void send(std::string pkg, std::function<void(std::string& res)> _cb);

	void qsend(std::string pkg, const std::function<void(boost::json::object& pt)>& _cb = nullptr);

	void qread();

	std::function<void(boost::json::object& pt)> process_actions_fn;

};

//void read_socket_task(boost::asio::ip::tcp::socket* socket);

//void connection_task();

#endif /* connections_hpp */
