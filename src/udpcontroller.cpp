#include "udpcontroller.h"
#include <iostream>
//#include <boost/array.hpp>
#include <boost/asio.hpp>

#define BUFFSIZE (64 * 1024)

using namespace std;
using boost::asio::ip::udp;


UdpController::UdpController(
	const boost::asio::any_io_executor& executor,
	udp::endpoint& local_endpoint,
	const string& _local_id
):
	socket(executor),
	local_id(_local_id)
{

	this->socket.open(udp::v4());
	this->socket.bind(local_endpoint);
	this->start_receive();

}


boost::asio::ip::udp::socket& UdpController::get_socket() {
	
	return this->socket;

}


void UdpController::start_receive() {

	//std::shared_ptr<char[]> recv_buff(new char[BUFFSIZE]);
	auto recv_buff = new char[BUFFSIZE];

	auto _remote_endpoint = make_shared<udp::endpoint>();

	//cout << "!! WAITING FOR NEXT UDP PKG IN BACKGROUND... " << endl;

	auto handler = [&, recv_buff, _remote_endpoint] (const boost::system::error_code& error, size_t bytes_transferred) {

		//string dgram = string(recv_buff.get(), bytes_transferred);
		string dgram = string(recv_buff, bytes_transferred);
		cout << "<- UDP | " << *_remote_endpoint << " | " << bytes_transferred << "B" << endl;

		try {

			auto json = boost::json::parse(dgram);
			
			//cout << "DECODED: " << json << endl;
			string sender_id = json.as_object()["sender_id"].as_string().c_str();
			string ep = _remote_endpoint->address().to_string() + ":" + to_string(_remote_endpoint->port()) + ":" + sender_id;
			
			auto ch = this->channels.find(ep);

			if (ch == this->channels.end()) {
				string type = json.as_object()["type"].as_string().c_str();
				if (type != "handshake") {
					throw runtime_error("First udp pkg received was not of type handshake");
				}
		
				//UdpChannelController new_ch(this, *_remote_endpoint);
				cout << "!! A new channel was created for this endpoint!" << endl;
				//this->channels.insert(pair(ep, move(new_ch)));
				//this->channels.emplace(ep, forward_as_tuple(this, _remote_endpoint));
				this->channels.emplace(ep, make_unique<UdpChannelController>(this, *_remote_endpoint, sender_id));
				auto& ch = this->channels.find(ep)->second;
				ch->send_handshake_acknowledgement();
				if (this->on_new_channel != nullptr) {
					this->on_new_channel(*ch);
				}
			} else {
				ch->second->handle_pkg(json);
			}

		} catch (exception& e) {
			cerr << "Cannot decode pkg: " << e.what() << endl;
			cerr << "RECV_DATA: " << dgram << endl;
		}

		this->start_receive();

	};

	//this->socket.async_receive_from(boost::asio::buffer(recv_buff.get(), BUFFSIZE), *_remote_endpoint, handler);
	this->socket.async_receive_from(boost::asio::buffer(recv_buff, BUFFSIZE), *_remote_endpoint, handler);

}


unique_ptr<UdpChannelController>& UdpController::create_channel(boost::asio::ip::udp::endpoint& remote_ep, const string& id) {
	
	cout << "Creating UDP channel [ID: " << id << ", EP: " << remote_ep << "]" << endl;
	//UdpChannelController new_ch(this, remote_ep);
	string ch_key = remote_ep.address().to_string() + ":" + to_string(remote_ep.port()) + ":" + id;
	
	//cout << "Saving UDP channel... " << endl;
	//this->channels.insert(pair(ep, move(new_ch)));
	//this->channels.emplace(ep, move(new_ch));
	this->channels.emplace(ch_key, make_unique<UdpChannelController>(this, remote_ep, id));
	
	cout << "Done! " << endl;
	return this->channels.find(ch_key)->second;
	//return this->channels[remote_ep];

} 


UdpChannelController::UdpChannelController(
	UdpController* _udp_controller,
	udp::endpoint& _remote_endpoint,
	const string& _remote_id
):
	udp_controller(_udp_controller),
	remote_endpoint(_remote_endpoint),
	remote_id(_remote_id)
{

	this->next_pkg_id = 1;
	this->last_handled_pkg_id = 0;

}



const std::string& UdpChannelController::get_remote_id() {

	return this->remote_id;

}


void UdpChannelController::handle_pkg(boost::json::value& json) {

	auto pkg = json.as_object();
	auto type = pkg["type"].as_string();

	if (type == "acknowledge") {

		this->confirmed_handshake = true;

		uint64_t id = pkg["id"].as_int64();
		cout << "<- UDP ACK " << id << endl;
		//remove unconfirmed_pkg
		for (auto it = this->unconfirmed_pkgs.begin(); it < this->unconfirmed_pkgs.end(); it++) {
			if (id == *it) {
				this->unconfirmed_pkgs.erase(it, it + 1);
				break;
			}
		}

	} else if (type == "data") {

		//cout << "sending acknowledgement... " << endl;
		uint64_t id = pkg["id"].as_int64();//throws exception if i try to get value as uint64...
		this->send_acknowledgement(id); 

		if (id > this->last_handled_pkg_id) {
			//cout << "pushing to pkgs_buffer... " << endl;
			this->pkgs_buffer[id] = pkg;
			//cout << "yield... " << endl;
			this->yield_pkgs_buffer();
		} else {
			//ignore this package, it was already processed!
		}

	} else {

		cerr << "Unsupported UDP pkg type: " << type << endl;

	}

}


void UdpChannelController::yield_pkgs_buffer() {

	if (this->process_actions_fn == nullptr) {
		return;
	}

	uint64_t next_pkg_to_handle_id = this->last_handled_pkg_id + 1;

	auto pkg_it = this->pkgs_buffer.find(next_pkg_to_handle_id);

	if (pkg_it == this->pkgs_buffer.end()) {
		return;
	}
	
	auto& pkg = pkg_it->second;

	cout << "[] ---> yielding pkg " << next_pkg_to_handle_id;
	auto obj = boost::json::parse(pkg["data"].as_string()).as_object();
	this->process_actions_fn(obj);
	this->last_handled_pkg_id = next_pkg_to_handle_id;
	this->pkgs_buffer.erase(next_pkg_to_handle_id);//, pkg_it + 1);
	cout << " | REM: " << this->pkgs_buffer.size() << endl;
	this->yield_pkgs_buffer();

}


void UdpChannelController::send_acknowledgement(uint64_t id) {

	boost::json::object obj = {
		{"type", "acknowledge"},
		{"id", id},
		{"sender_id", this->udp_controller->local_id}
	};
	
	string pkg = boost::json::serialize(obj);

	auto handler = [&] (const boost::system::error_code& error, std::size_t bytes_transferred) {

		if (error) {
			throw std::runtime_error(error.message());
		}

	};

	//cout << ">> UDP ACK | " << this->remote_endpoint <<  " | " << id << endl;
	
	udp::socket& sock = this->udp_controller->get_socket();

	sock.async_send_to(boost::asio::buffer(pkg), this->remote_endpoint, handler);

}


uint64_t UdpChannelController::get_new_pkg_id() {

	if (!this->confirmed_handshake) {
		throw runtime_error("ERROR: trying to generate pkg id before handshake");
	}

	uint64_t id = this->next_pkg_id++;

	this->unconfirmed_pkgs.push_back(id);

	return id;

}


void UdpChannelController::_send(const string& data, uint64_t id, uint64_t count) {

	udp::socket& sock = this->udp_controller->get_socket();

	auto sendbuf = make_shared<string>(data);

	auto t = make_shared<boost::asio::steady_timer>(sock.get_executor());

	auto check = [this, id, sendbuf, count, t] (const boost::system::error_code& error) {

		//comprobar si se ha recibido acuse de recibo en el otro listener y en caso negativo enviar de nuevo!
		for (auto & k_id: this->unconfirmed_pkgs) {
			if (id == k_id) {
				if (count > 100) {
					throw runtime_error("UDP Channel dropped: " + this->remote_id);
					return; // stop trying after 100 tries
				}
				//cerr << "!! No ack received for pkg " << id << " --> sending again!" << endl;
				this->_send(*sendbuf, id, count + 1);
				return;
			}
		}
		//cout << "T executor: " << t->get_executor() << endl;

	};

	auto handler = [this, check, count, t] (const boost::system::error_code& error, std::size_t bytes_transferred) {

		cout << "-> UDP | " << this->remote_endpoint << " | " << bytes_transferred << "B";
		if (count > 0) {
			cout << " | !! T" << count << endl;
		} else {
			cout << endl;
		}
		//cerr << " entering _send handler " << endl;
		if (error) {
			throw std::runtime_error("ERROR: (_send handler) " + error.message());
		}

		t->expires_after(boost::asio::chrono::milliseconds(10 + count * 2));
  		t->async_wait(check);
		
	};

	sock.async_send_to(boost::asio::buffer(*sendbuf), this->remote_endpoint, handler);

}


void UdpChannelController::send(const string& data) {

	uint64_t id = this->get_new_pkg_id();

	boost::json::object obj = {
		{"id", id},
		{"type", "data"},
		{"sender_id", this->udp_controller->local_id},
		{"data", data}
	};

	string dgram = boost::json::serialize(obj);

	this->_send(dgram, id);

}


void UdpChannelController::send_handshake() {

	if (this->confirmed_handshake) {
		return;
	}

	boost::json::object obj = {
		{"id", 0},
		{"type", "handshake"},
		{"sender_id", this->udp_controller->local_id}
	};

	string dgram = boost::json::serialize(obj);

	this->_send(dgram, 0);

}


void UdpChannelController::send_handshake_acknowledgement() { 

	cout << "-> UDP ACK handshake " << this->remote_id << endl;
	this->send_acknowledgement(0);
	this->confirmed_handshake = true;

}

	
