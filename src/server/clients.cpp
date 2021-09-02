
#include "clients.h"
#include "server.h"
#include "../netutils.h"

#include <iostream>
#include <stdlib.h>

using namespace std;
using namespace boost::asio::ip;


int Client::count_instances = 0;

Client::Client(
	boost::asio::io_service& io_service, 
	tcp::acceptor& acceptor
): 
    socket(io_service)
{

    this->id_client = count_instances++;
	acceptor.accept(socket, this->conn_err);

	if(this->conn_err){
		this->dead = true;
		cout << "Error opening socket: " << this->conn_err << endl;
	} else {
		this->connected = true;
	}

}


Client::~Client(){

	this->socket.cancel();
	this->socket.close();
	cout << "BYE BYE" << endl;

}


void Client::process_request(std::string request){
	
	boost::json::value q = boost::json::parse( request );

	if ( !q.is_object() ){

		cerr << "Not valid JSON" << endl << request << endl;
		return;

	}
    
    boost::json::object evt = q.get_object();

	auto evtType = evt["type"].get_string();

	if(evtType == "ping"){

		evt["type"] = "pong";
		this->qsend(boost::json::serialize(evt));

	} else {

		if( this->on_pkg_received != nullptr){

			this->on_pkg_received(evt);

		}

	}

}


void Client::qsend(std::string pkg, bool ignore_busy){

	if(this->busy && !ignore_busy){
		this->pkg_queue.push(pkg);
		return;
	}

	//cout << "sending " << pkg.length() << " bytes" << endl;
	this->busy = true;

	auto handler = boost::bind(&Client::handle_qsent_content, this,
							   boost::asio::placeholders::error(),
							   boost::asio::placeholders::bytes_transferred());

	if(pkg.length() > 1024 * 64){ //max 64kB (me lo estoy inventando a ver si cuela...)

		this->pending_data = pkg.substr (1024 * 64);
		pkg = pkg.substr (0, 1024 * 64);

	} else {

		this->pending_data = "";
		pkg = pkg + "\r\n\r\n";

	}

	boost::asio::async_write(socket, boost::asio::buffer(pkg), handler);

	this->pkgs_sent++;
	//std::cout << " S:" << pkgs_sent << endl;

}


void Client::handle_qsent_content( const boost::system::error_code& error, std::size_t bytes_transferred){

	if(error){
		cout << "Error occurred (SEND)[C" << this->id_client << "]: " << error << endl;
		this->dead = true;
		return;
	}

	if (this->pending_data.length() > 0){
		cout << "bunch " << bytes_transferred << " (" << this->pending_data.length() << " remaining)"<< endl;
		this->qsend(this->pending_data, true);
		return;
	}

	this->busy = false;

	if(!this->pkg_queue.empty()){
		this->qsend(this->pkg_queue.front());
		this->pkg_queue.pop();
	}

}


bool Client::is_dead(){
	//return !this->socket.is_open();
	return this->dead;
}


bool Client::is_connected(){
	return this->connected;
}


void Client::async_wait_for_data(){

	auto handler = boost::bind(&Client::handle_read_content, this,
								boost::asio::placeholders::error(),
								boost::asio::placeholders::bytes_transferred());

	this->socket.async_read_some( boost::asio::buffer(read_buffer, 1024), handler);
	//cout << "Waiting for data ... " << endl;

}


void Client::handle_read_content(const boost::system::error_code& error, std::size_t bytes_transferred){

	if(error){

        cout << "Error occurred (READ)[C" << this->id_client << "]: " << error << endl;
		this->dead = true;
        return;
	}
	//std::cout << "Data received[C" << this->id_client << "]: ";


	std::string data((char*)read_buffer, bytes_transferred);
	data = read_remainder + data;

	string pkg;
	while((pkg = extract_pkg(data)) != ""){
		pkgs_recv ++;
		//std::cout << " R:" << pkgs_recv << endl;

		//boost::thread(boost::bind(&Client::process_request, this, pkg, req_id));
		this->process_request(pkg);
	}
	
	read_remainder = data;

	this->async_wait_for_data();

}
