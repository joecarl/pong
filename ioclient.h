//
//  lumper.h
//  spl
//
//  Created by Joe on 31/1/15.
//  Copyright (c) 2015 Copinstar. All rights reserved.
//


#ifndef spl_lumper_h
#define spl_lumper_h

#include <iostream>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
using namespace std;

class ioClient{
    int timeout;
    boost::thread *io_th, *send_th;
    boost::asio::streambuf buff;
    vector<string> pkgout_queue;
    vector<string> pkgin_queue;
    boost::asio::io_service io_service;
    boost::asio::deadline_timer deadline;
    boost::asio::ip::tcp::socket socket;
    void handle_connect(const boost::system::error_code& ec);
    void handle_read(const boost::system::error_code& e, std::size_t size);
    void check_deadline();
    void io_task();
    void send_task();
public:
    bool stopped,connected;
    
    ioClient():deadline(io_service),socket(io_service){connected=stopped=0;}
    void Reset(){connected=stopped=0;io_service.stop();io_service.reset();socket.close();}
    void Connect(string ip, string port);
    const char* FetchPacket();
    int SendPacket(string pkg);
};

#endif
