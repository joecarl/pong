
#include "clients.h"
#include "server.h"
#include "../utils.h"

#include <iostream>
#include <stdlib.h>

using namespace std;
using namespace boost::asio::ip;


uint64_t Client::count_instances = 0;

Client::Client(boost::asio::ip::tcp::socket& _socket): 
	socket(std::move(_socket))
{

	this->id_client = count_instances++;

	boost::json::object setup = {
		{"type", "set_client_id"},
		{"client_id", "C" + to_string(this->id_client)}
	};

	this->qsend(boost::json::serialize(setup));

}


Client::~Client() {

	this->socket.cancel();
	this->socket.close();

}


uint64_t Client::get_id() { 
	
	return this->id_client;

}


void Client::set_udp_channel(UdpChannelController& ch) {
	
	//cout << "!! Setting UdpChannelController" << endl;

	this->udp_channel = &ch;

	this->udp_channel->process_actions_fn = [this] (boost::json::object obj) {

		if (this->on_pkg_received != nullptr) {

			this->on_pkg_received(obj);

		}

	};


}


void Client::add_event_listener(const string &evt_name, const std::function<void()> &fn) {
	
	this->evt_listeners.push_back({evt_name, fn});

}


void Client::trigger_event(const string& evt_name) {
	
	for (auto &ev: this->evt_listeners) {
		if (ev.evt_name == evt_name) {
			ev.cb();
		}
	}

}


void Client::process_request(const string& request) {
	
	boost::json::value q = boost::json::parse(request);

	if (!q.is_object()) {

		cerr << "Not valid JSON" << endl << request << endl;
		return;

	}
	
	boost::json::object evt = q.get_object();

	auto evt_type = evt["type"].get_string();

	if (evt_type == "ping") {

		evt["type"] = "pong";
		this->qsend(boost::json::serialize(evt));

	} else {

		if (this->on_pkg_received != nullptr) {

			this->on_pkg_received(evt);

		}

	}

}


void Client::qsend_udp(const string& pkg) {

	if (this->udp_channel == nullptr) {
		cout << "TCP fallback" << endl;
		this->qsend(pkg);
		return;
	}
	
	this->udp_channel->send(pkg);

}


void Client::qsend(string pkg, bool ignore_busy) {

	if (this->busy && !ignore_busy) {
		this->pkg_queue.push(pkg);
		return;
	}

	//cout << "sending " << pkg.length() << " bytes" << endl;
	this->busy = true;

	auto handler = boost::bind(
		&Client::handle_qsent_content, 
		this,
		boost::asio::placeholders::error(),
		boost::asio::placeholders::bytes_transferred()
	);

	if (pkg.length() > 1024 * 64) { //max 64kB (me lo estoy inventando a ver si cuela...)

		this->pending_data = pkg.substr(1024 * 64);
		pkg = pkg.substr(0, 1024 * 64);

	} else {

		this->pending_data = "";
		pkg = pkg + "\r\n\r\n";

	}

	boost::asio::async_write(socket, boost::asio::buffer(pkg), handler);

	this->pkgs_sent++;
	//std::cout << " S:" << pkgs_sent << endl;

}


void Client::handle_qsent_content(const boost::system::error_code& error, std::size_t bytes_transferred) {

	if (error) {
		cout << "Error occurred (SEND)[C" << this->id_client << "]: " << error << endl;
		this->dead = true;
		this->trigger_event("onDrop");
		return;
	}

	if (this->pending_data.length() > 0) {
		cout << "bunch " << bytes_transferred << " (" << this->pending_data.length() << " remaining)" << endl;
		this->qsend(this->pending_data, true);
		return;
	}

	this->busy = false;

	if (!this->pkg_queue.empty()) {
		this->qsend(this->pkg_queue.front());
		this->pkg_queue.pop();
	}

}


bool Client::is_dead() {
	//return !this->socket.is_open();
	return this->dead;
}



void Client::async_wait_for_data() {

	auto handler = boost::bind(
		&Client::handle_read_content, 
		this,
		boost::asio::placeholders::error(),
		boost::asio::placeholders::bytes_transferred()
	);

	this->socket.async_read_some(boost::asio::buffer(read_buffer, 1024), handler);
	//cout << "Waiting for data ... " << endl;

}


void Client::handle_read_content(const boost::system::error_code& error, std::size_t bytes_transferred) {

	if (error) {

		cout << "Error occurred (READ)[C" << this->id_client << "]: " << error << endl;
		this->dead = true;
		this->trigger_event("onDrop");
		return;
	}
	//std::cout << "Data received[C" << this->id_client << "]: ";

	string data((char*) read_buffer, bytes_transferred);
	data = read_remainder + data;

	string pkg;
	while ((pkg = extract_pkg(data)) != "") {
		pkgs_recv ++;
		//std::cout << " R:" << pkgs_recv << endl;

		this->process_request(pkg);
	}
	
	read_remainder = data;

	this->async_wait_for_data();

}
