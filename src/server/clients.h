
#ifndef CLIENTS_H
#define CLIENTS_H

#include "../udpcontroller.h"
#include <boost/json.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <queue>
#include <vector>

struct EventListener {
	std::string evt_name;
	std::function<void()> cb;
};

class Client {

	static uint64_t count_instances;

	uint64_t pkgs_recv = 0;

	uint64_t pkgs_sent = 0;

	uint64_t id_client = 0;

	bool busy = false;

	bool dead = false;
	
	bool app_validated = false;

	UdpChannelController* udp_channel;

	boost::asio::ip::tcp::socket socket;

	std::vector<EventListener> evt_listeners;
	
	std::queue<std::string> pkg_queue;

	std::string pending_data = "";

	unsigned char read_buffer[1024];

	std::string read_remainder = "";

	void handle_read_content(const boost::system::error_code& error, std::size_t bytes_transferred);

	void handle_qsent_content(const boost::system::error_code& error, std::size_t bytes_transferred);

public:

	Client(boost::asio::ip::tcp::socket& _socket);
	
	~Client();
	
	uint64_t get_id();

	void set_udp_channel(UdpChannelController& ch);

	void async_wait_for_data();

	void add_event_listener(const std::string& evt_name, const std::function<void()> &fn);
	
	void trigger_event(const std::string& evt_name);
	
	void qsend(std::string pkg, bool ignore_busy = false);
	
	void qsend_udp(const std::string& pkg);

	bool is_dead();

	void process_request(const std::string& request);
	
	std::function<void(boost::json::object& pt)> on_pkg_received;

};

#endif
