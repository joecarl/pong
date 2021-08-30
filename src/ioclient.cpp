//
//  lumper.cpp
//  spl
//
//  Created by Joe on 31/1/15.
//  Copyright (c) 2015 Copinstar. All rights reserved.
//

#include "ioclient.h"
#include <boost/asio/deadline_timer.hpp>
using boost::asio::ip::tcp;

void ioClient::Connect(string ip, string port){
    try{
        timeout = 1000;
        cout << "Configurando parámetros de conexión..." << endl;
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(ip, port);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        boost::asio::ip::tcp::endpoint endpoint=endpoint_iterator->endpoint();//(boost::asio::ip::address::from_string(ip), atoi(port.c_str()));
        
        deadline.expires_from_now(boost::posix_time::millisec(timeout * 5));
        socket.async_connect(endpoint, 
			boost::bind(&ioClient::handle_connect, this, 
				boost::asio::placeholders::error()
			)
		);
        deadline.async_wait(boost::bind(&ioClient::check_deadline, this));
        io_th = new boost::thread(boost::bind(&ioClient::io_task, this));
    }

    catch(std::exception& e){
        stopped = true;
        std::cerr << e.what() << std::endl;
    }
}

const char* ioClient::FetchPacket(){
    
	if (pkgin_queue.size() > 0){

        string out = pkgin_queue[0];
        for (unsigned int i = 0; i < pkgin_queue.size() - 1; i++){
            pkgin_queue[i] = pkgin_queue[i + 1];
        }
        pkgin_queue.pop_back();
        return out.c_str();
    }
    else return NULL;

}

int ioClient::SendPacket(string pkg){

    pkgout_queue.push_back(pkg);
    return 1;

}

void ioClient::send_task(){
    boost::system::error_code error;
    while (connected) {
        //usleep(1000);
        boost::this_thread::sleep(boost::posix_time::milliseconds(1));
        if(pkgout_queue.size() > 0 && connected){
            boost::asio::write(socket, boost::asio::buffer(pkgout_queue[0].c_str(), pkgout_queue[0].length()), error);
            for (unsigned int i = 0; i < pkgout_queue.size() - 1; i++)
                pkgout_queue[i] = pkgout_queue[i + 1];
            pkgout_queue.pop_back();
        }
    }
}

void ioClient::handle_read(const boost::system::error_code& ec, std::size_t size){
    
	if (!ec){
    
	    stringstream aux;
        aux << &buff;
        string buff_str = aux.str();
        buff_str.resize(size);
        pkgin_queue.push_back(buff_str);
        cout << "Recibido: " << buff_str;
        boost::asio::async_read_until(socket, buff, "\r\n", 
			boost::bind(&ioClient::handle_read, this, 
				boost::asio::placeholders::error(),
				boost::asio::placeholders::bytes_transferred()
			)
		);
    
	} else {
    
	    if(ec == boost::asio::error::eof){
            stopped=1;
            deadline.cancel();
        } else {
            std::runtime_error err("error de lectura");
            throw err;
        }
    }
}

void ioClient::handle_connect(const boost::system::error_code& ec) {
    
	if (!ec) {

        cout << "Conectado al servidor!" << endl;
        connected = true;
        deadline.expires_from_now(boost::posix_time::millisec(timeout));
        boost::asio::async_read_until(socket, buff, "\r\n", 
			boost::bind(&ioClient::handle_read, this, 
				boost::asio::placeholders::error(),
				boost::asio::placeholders::bytes_transferred()
			)
		);
        
        send_th = new boost::thread(boost::bind(&ioClient::send_task, this));
    } else {
        stopped = true;
        std::runtime_error err("error de conexion");
        throw err;
    }

}

 void ioClient::Reset(){
	connected = stopped = 0;
	io_service.stop();
	io_service.reset();
	socket.close();
}

void ioClient::check_deadline(){

    if(stopped) return;

    if (deadline.expires_at() <= boost::asio::deadline_timer::traits_type::now()){
        cout << "Tiempo de espera agotado." << endl;
        std::runtime_error err("timeout excedido");
        throw err;
    }

    deadline.expires_from_now(boost::posix_time::pos_infin);
    deadline.async_wait(boost::bind(&ioClient::check_deadline, this));

}

void ioClient::io_task(){
    
	try {
        cout << "Esperando conexión..." << endl;
        io_service.run();
    } catch (std::exception& e) {
        connected = false;
        std::cerr << e.what() << std::endl;
    }
    
}





