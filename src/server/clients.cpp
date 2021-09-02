//
//  clients.cpp
//  mysql_test
//
//  Created by Joe on 10/9/18.
//  Copyright © 2018 Joe. All rights reserved.
//

#include "clients.h"
#include "server.h"
#include "../netutils.h"

#include <iostream>
#include <stdlib.h>

using namespace std;
using namespace boost::asio::ip;



int Client::count_instances = 0;

Client::Client(boost::asio::io_service& io_service, tcp::acceptor& acceptor): 
    socket(io_service)//, 
    //pl()
{

	//world::players[world::playersQuant] = &pl;
	//world::playersQuant ++;

	for(int i = 0; i < 1000/*ALLEGRO_KEY_MAX*/; i++) keyb[i] = false;
	for(int i = 0; i < 10; i++) mouse[i] = false;

    this->id_client = count_instances++;
	acceptor.accept(socket, this->conn_err);
	if(this->conn_err){
		this->dead = true;
		cout << "Error opening socket: " << this->conn_err << endl;
	}
	else{
		this->connected = true;
	}
	//this->socket = socket_;
}

Client::~Client(){

	this->socket.cancel();
	this->socket.close();
	cout << "BYE BYE" << endl;

}


void Client::process_request(std::string request){
	//let's parse query

	boost::json::value q = boost::json::parse( request );
	if ( !q.is_object() ){
		cerr << "Error parsing the request" << endl;
		cerr << request << endl;
		return;
	} else {
		//cout << q << endl;
	}
    
    boost::json::object obj = q.get_object();

	string action = obj["action"].get_string().c_str();
	if(action == "ping"){
		this->qsend("{\"response\":\"ping\", \"result\":\"ok\"}");
	}
    
    /*
	else if(action == "mouse_move"){
		int inc_mouse_x = atoi(getstr(q, "dx").c_str());
		int inc_mouse_y = atoi(getstr(q, "dy").c_str());
		pl.process_mouse_move(inc_mouse_x, inc_mouse_y);
	}

	else if(action == "mouseup"){
		int btn = atoi(getstr(q, "key").c_str());
		this->mouse[btn] = false;
	}

	else if(action == "mousedown"){
		int btn = atoi(getstr(q, "key").c_str());
		this->mouse[btn] = true;
	}
*/
	else if(action == "keyup"){
		int keycode = obj["key"].get_int64();//atoi(getstr(q, "key").c_str());
		this->keyb[keycode] = false;
	}

	else if(action == "keydown"){
		int keycode = obj["key"].get_int64();//atoi(getstr(q, "key").c_str());
		this->keyb[keycode] = true;
	}

	else{
		//this->actions.push(q);
		this->process_stage_action(obj);
	}

	//this->process_actions_fn(q, this);

}
/*
void Client::process_stage_action(boost::json::object& pt){
	if(this->stage == 0){
		stage0_process_action(this, pt);
	}
	else if (this->stage == 1){
		stage1_process_action(this, pt);
	}
}
*/
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
	//boost::asio::write(socket, boost::asio::buffer(pkg + "\r\n\r\n"));
	if(pkg.length() > 1024 * 64){ //max 64kB (me lo estoy inventando a ver si cuela...)
		this->pending_data = pkg.substr (1024 * 64);
		pkg = pkg.substr (0, 1024 * 64);

	} else {
		this->pending_data = "";
		pkg = pkg + "\r\n\r\n";
	}
	boost::asio::async_write(socket, boost::asio::buffer(pkg), handler);
	//socket.async_write_some(boost::asio::buffer(pkg + "\r\n\r\n"), handler);
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

	//std::string buff = "ADDFSFGDSGSDF GSDFG SDFGDSFGSDFGSDFGSD ";
	//this->socket.async_write_some(boost::asio::buffer(buff), handler);
}
