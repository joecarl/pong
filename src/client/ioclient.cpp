
#include "ioclient.h"
#include "../udpcontroller.h"
#include "../utils.h"

#include <iostream>
#include <chrono>
#include <thread>

using namespace std;

using boost::asio::ip::udp;
namespace asio = boost::asio;

IoClient::IoClient(): 
	socket(io_context),
	udp_controller(nullptr),
	udp_channel(nullptr)
{

}


int64_t time_ms() {
	
	/*
	auto now = std::chrono::system_clock::now();
	auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	return now_ms.time_since_epoch().count()
	*/

	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
	).count();

}


void IoClient::connect(const string& host, unsigned short port) {

	this->connection_state = CONNECTION_STATE_CONNECTING;
	
	boost::thread([=] {

		try {

			// Creating a query.
			asio::ip::tcp::resolver::query resolver_query(
				host, to_string(port), 
				asio::ip::tcp::resolver::query::numeric_service
			);

			// Creating a resolver.
			asio::ip::tcp::resolver resolver(io_context);

			// Resolve query
			cout << "Resolving " << host << " ..." << endl;
			asio::ip::tcp::resolver::iterator it = resolver.resolve(resolver_query);

			asio::ip::tcp::endpoint endpoint = it->endpoint();
			/*
			//Otra forma de hacerlo es iterando sobre todas las resoluciones encontradas
			asio::ip::tcp::resolver::iterator it_end;
			asio::ip::tcp::endpoint endpoint;

			for (; it != it_end; it++) {
				endpoint = it->endpoint(); //nos quedaremos con el último
				cout << "Found " << endpoint << endl;
			}
			*/
			/*
			//otra forma también es especificar IP y puerto directamente
			endpoint.address(asio::ip::make_address(addr));
			endpoint.port(port);
			*/
			cout << "Trying " << endpoint << " ..." << endl;

			socket.connect(endpoint);

			
			cout << "Connected to " << socket.remote_endpoint() << " !" << endl;

			this->connection_state = CONNECTION_STATE_CONNECTED;

			this->current_host = host;

			cout << "Connected!" << endl;

			boost::thread([=] {

				while (true) {

					//cout << "milliseconds since epoch: " << time_ms() << endl;

					boost::json::object pingPkg = {
						{"type", "ping"},
						{"ms", time_ms()}
					};

					this->qsend(boost::json::serialize(pingPkg));
					
					std::this_thread::sleep_for(std::chrono::seconds(1));

				}

			});

			qread();

			io_context.run();

		} catch (std::exception &e) {

			cout << "Error occurred[C" << this->id_client << "]: " << e.what() << endl;
			this->connection_state = CONNECTION_STATE_DISCONNECTED;

		}

	});

}

IoClient::~IoClient() {

	this->io_context.stop();

	std::this_thread::sleep_for(std::chrono::seconds(1));
	
}

int IoClient::get_state() {

	return this->connection_state;

}


void IoClient::setup_udp(string& local_id) {

	auto tcp_local_ep = this->socket.local_endpoint();
	auto tcp_remote_ep = this->socket.remote_endpoint();

	udp::endpoint local_endpoint(tcp_local_ep.address(), tcp_local_ep.port());
	udp::endpoint remote_endpoint(tcp_remote_ep.address(), tcp_remote_ep.port());
	this->udp_controller = new UdpController(this->socket.get_executor(), local_endpoint, local_id);
	this->udp_channel = this->udp_controller->create_channel(remote_endpoint, "SERVER").get();

	this->udp_channel->send_handshake();

	this->connection_state = CONNECTION_STATE_CONNECTED_FULL;

}


void IoClient::qsend_udp(const std::string& pkg, const std::function<void(boost::json::object& pt)>& _cb) {
	
	if (this->udp_channel == nullptr) {
		cerr << "Cannot sent pkg, upd channel not established yet" << endl;
		return;
	}

	if (_cb != nullptr) {
		this->save_cb(pkg, _cb);
	}

	this->udp_channel->send(pkg);

	pkgs_sent++;

}


void IoClient::save_cb(const std::string& pkg, const std::function<void(boost::json::object& pt)>& _cb) {

	boost::json::object obj = boost::json::parse(pkg).get_object();

	//std::string action = obj["action"];
	std::string action = obj["action"].get_string().c_str();//pt.get<std::string>("action");
	
	if (action.length() > 0) {
		auto t_start = std::chrono::high_resolution_clock::now();

		cbs.push_back({action, _cb, t_start});
	}

}
	

void IoClient::qsend(std::string pkg, const std::function<void(boost::json::object& pt)>& _cb) {
	
	if (this->busy) {
		this->pkg_queue.push({pkg, _cb});
		return;
	}

	this->busy = true;
/*
	auto handler = boost::bind(&IoClient::handle_qsent_content, this,
							   asio::placeholders::error(),
							   asio::placeholders::bytes_transferred());
*/
	pkg += "\r\n\r\n";

	if (_cb != nullptr) {
		
		this->save_cb(pkg, _cb);

	}

	auto handler = [this] (const boost::system::error_code& error, std::size_t bytes_transferred) {
		this->handle_qsent_content(error, bytes_transferred);
	};

	asio::async_write(socket, asio::buffer(pkg), handler);
	
	pkgs_sent++;

}

void IoClient::qread() {
/*
	auto handler = boost::bind(&IoClient::handle_qread_content, this,
							   asio::placeholders::error(),
							   asio::placeholders::bytes_transferred());
*/
	auto handler = [this] (const boost::system::error_code& error, std::size_t bytes_transferred) {
		this->handle_qread_content(error, bytes_transferred);
	};

	socket.async_read_some(asio::buffer(read_buffer, 1024), handler);

}

void IoClient::handle_qsent_content(const boost::system::error_code& error, std::size_t bytes_transferred) {

	this->busy = false;

	if (error) {
		
		throw std::runtime_error(error.message());

	}

	if (!this->pkg_queue.empty()) {
		this->qsend(this->pkg_queue.front().pkg, this->pkg_queue.front()._cb);
		this->pkg_queue.pop();
	}

}


void IoClient::set_process_actions_fn(const std::function<void(boost::json::object& pt)>& _fn) {
	
	this->process_actions_fn = _fn;

	this->udp_channel->process_actions_fn = _fn;

}


void IoClient::handle_qread_content(const boost::system::error_code& error, std::size_t bytes_transferred) {

	if (error) {
		
		throw std::runtime_error(error.message());

	}

	std::string data((char*)read_buffer, bytes_transferred);
	
	data = read_remainder + data;
	std::string pkg;

	while ((pkg = extract_pkg(data)) != "") {
		pkgs_recv ++;
		//std::cout << " R:" << pkgs_recv << endl;

		boost::json::value pt;
/*
		if (this->wait_for_binary) {
			this->wait_for_binary = false;
			//pt = this->wait_for_binary_pt;
			pt["action"] = "binary_transfer";
			pt["data"] = pkg;

			this->process_actions_fn(pt);

		} else {
*/
			pt = boost::json::parse(pkg);

			if (pt.is_object()) {

				boost::json::object obj = pt.get_object();
				//cout << pkg << endl;
				/*
				if (pt["binary_transfer"]) {//as bool
					this->wait_for_binary = true;
					this->wait_for_binary_pt = pt;
				}
				*/

				/*
				boost::json::value resp = obj["response"];
				if (!resp.is_null()) { //as string
					std::string response_name = resp.get_string().c_str();//.get<std::string>("response");

					for (std::size_t i = 0; i < cbs.size(); i++) {
						if (cbs[i].name == response_name) { //si hay dos con el mismo nombre que pasa?
							cbs[i].cb(obj);

							cbs.erase(cbs.begin() + i);
							if (cbs.size() > 0) {
								cout << "CM remaining: " << cbs.size() << endl;
							}

							break;
						}
					}

				}
				*/

				if (obj["type"].is_string()) {

					if (obj["type"] == "set_client_id") {
						
						string id = obj["client_id"].as_string().c_str();
						cout << "SETTING UP UDP. CLIENT ID: " << id << endl;
						this->setup_udp(id);

					} else if (obj["type"] == "pong") {
						
						this->ping_ms = time_ms() - obj["ms"].as_int64();
						//std::cout << "PING: " << this->ping_ms << "ms" << endl;

					} else if (this->process_actions_fn != nullptr) {

						//std::cout << "ACCION!!!" << std::endl;
						this->process_actions_fn(obj);

					}
					
				}

			} else {

				cout << "error parsing: " << pkg << endl;

			}
//		}
	}
	read_remainder = data;
	//cout << read_remainder << endl;
	qread();
}
