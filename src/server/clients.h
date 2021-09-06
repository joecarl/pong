
#ifndef clients_hpp
#define clients_hpp

#include <boost/json.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <queue>
#include <vector>

struct EventListener{
	std::string evtName;
	std::function<void()> cb;
};

class Client{

	//game vars
	int pkgs_recv = 0;
	int pkgs_sent = 0;

	std::vector<EventListener> evtListeners;
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


public:

	//conn vars
	boost::system::error_code conn_err;
	Client(boost::asio::io_service& io_service, boost::asio::ip::tcp::acceptor& acceptor);
	~Client();

	void async_wait_for_data();

	void addEventListener(const std::string &evtName, const std::function<void()> &fn);
	
	void triggerEvent(std::string evtName);
	
	void qsend(std::string pkg, bool ignore_busy = false);

	bool is_dead();

	bool is_connected();

	void process_request(std::string request);
	
	std::function<void(boost::json::object& pt)> on_pkg_received;

};

#endif
