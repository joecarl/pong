//
//  udpcontroller.h
//  pong
//
//  Created by Joe on 20/07/23.
//

#ifndef udp_controller_h
#define udp_controller_h

#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <memory>
#include <unordered_map>

class UdpChannelController;

/**
 * Creates a UDP socket controller. 
 */
class UdpController {

	/**
	 * The underlying socket which handles the data transmission
	 */
	boost::asio::ip::udp::socket socket;

	/**
	 * Map which stores every communication channel 
	 */
	std::unordered_map<std::string, std::unique_ptr<UdpChannelController>> channels;

	/**
	 * Will wait for new data asyncrhonously. If a package is recieved from an 
	 * unknown endpoint a new channel will be created automatically and the 
	 * on_new_channel function will be called
	 */
	void start_receive();

public:

	/**
	 * Stores the local ID used to "sign" every package sent, the receiver will 
	 * use it in addition to the endpoint to identify the channel
	 */
	const std::string local_id;

	/**
	 * Callback function which is called everytime a package is recieved from an
	 * unknown endpoint
	*/
	std::function<void(UdpChannelController& ch)> on_new_channel = nullptr;
	
	/**
	 * Creates the controller, binds it to the provided local endpoint and 
	 * stablishes the local ID
	 */
	UdpController(const boost::asio::any_io_executor& executor, boost::asio::ip::udp::endpoint& local_endpoint, const std::string& local_id);

	/**
	 * Returns the socket
	 */
	boost::asio::ip::udp::socket& get_socket();

	/**
	 * Creates a new channel identified by the provided remote endpoint and id.
	 * Note this function won't trigger `on_new_channel`
	 */
	std::unique_ptr<UdpChannelController>& create_channel(boost::asio::ip::udp::endpoint& remote_ep, const std::string& remote_id);

};


/**
 * Creates a controlled communication channel using an existing UDP socket
 */
class UdpChannelController {

	/**
	 * The udp controller instance which handles the socket
	 */
	UdpController* udp_controller;

	/**
	 * The remote enpoint which this channel will send/receive data to/from
	 */
	boost::asio::ip::udp::endpoint remote_endpoint;

	/**
	 * The ID used to verify the sender identity of packages received from
	 * `remote_endpoint`
	 */
	const std::string remote_id;

	/**
	 * Vector which stores all sent packages IDs whose acknowledgement was not
	 * received yet
	 */
	std::vector<uint64_t> unconfirmed_pkgs;

	/**
	 * All received packages are stored in this buffer, immediatelly all
	 * yieldable packages are yielded and removed form this buffer
	 */
	std::unordered_map<uint64_t, boost::json::object> pkgs_buffer;

	/**
	 * The numeric incremental ID used to number every sent package
	 */
	uint64_t next_pkg_id;

	/**
	 * The last received package which was yielded
	 */
	uint64_t last_handled_pkg_id;

	bool confirmed_handshake = false;

	void _send(const std::string& pkg, uint64_t id, uint64_t count = 0);

	void send_acknowledgement(uint64_t id);

	void yield_pkgs_buffer();

	uint64_t get_new_pkg_id();

public:

	const std::string& get_remote_id();

	void handle_pkg(boost::json::value& json);

	UdpChannelController(UdpController* _udp_controller, boost::asio::ip::udp::endpoint& _remote_endpoint, const std::string& _id);

	void send(const std::string& pkg);

	/**
	 * This function must be called by the client in order to start sending
	 * packages. When handshake acknowledgement is received communication is
	 * enabled and the `send` function can be called
	 */
	void send_handshake();

	/**
	 * This function must be called by the server in order to accept the
	 * connection
	 */
	void send_handshake_acknowledgement();

	std::function<void(boost::json::object& pt)> process_actions_fn;

};

#endif
