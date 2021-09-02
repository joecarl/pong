
#include "ioclient.h"
#include "../netutils.h"
#include <iostream>

using namespace std;

namespace asio = boost::asio;

IoClient::IoClient(): socket(io_service){

}

void IoClient::connect(string host, unsigned short port){

	this->connection_state = CONNECTION_STATE_CONNECTING;
	
	boost::thread( [=]{

		try {

			// Creating a query.
			asio::ip::tcp::resolver::query resolver_query(
				host, to_string(port), 
				asio::ip::tcp::resolver::query::numeric_service
			);
			
			// Creating a resolver.
			asio::ip::tcp::resolver resolver(io_service);

			// Resolve query
			cout << "Resolving " << host << " ..." << endl;
			asio::ip::tcp::resolver::iterator it = resolver.resolve(resolver_query);

			asio::ip::tcp::endpoint endpoint = it->endpoint();
			/*
			//Otra forma de hacerlo es iterando sobre todas las resoluciones encontradas
			asio::ip::tcp::resolver::iterator it_end;
			asio::ip::tcp::endpoint endpoint;

			for(; it != it_end; it++){
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

			cout << "Connected!" << endl;
			
			io_service.run();//will exit inmediately
			//quiza la lectura haya q hacerla en otro socket.. nose..
			qread();

		} catch (std::exception &e) {

			cout << "Error occurred[C" << this->id_client << "]: " << e.what() << endl;
			this->connection_state = CONNECTION_STATE_DISCONNECTED;

		}

	});

}

IoClient::~IoClient(){

	this->io_service.stop();

	//usleep(1000000);
	Sleep(1000);

}

int IoClient::get_state(){

	return this->connection_state;

}

void IoClient::qsend(std::string pkg, std::function<void(boost::json::object& pt)> _cb){
	//colas!!!!!
	if(this->busy){
		this->pkg_queue.push({pkg, _cb});
		return;
	}
	this->busy = true;

	auto handler = boost::bind(&IoClient::handle_qsent_content, this,
							   asio::placeholders::error(),
							   asio::placeholders::bytes_transferred());

	pkg += "\r\n\r\n";

	if(_cb != nullptr){
		
		boost::json::object obj = boost::json::parse(pkg).get_object();

		                     
		//std::string action = obj["action"];
		std::string action = obj["action"].get_string().c_str();//pt.get<std::string>("action");
		
		if(action.length() > 0){
			auto t_start = std::chrono::high_resolution_clock::now();

			cbs.push_back({action, _cb, t_start});
		}
	}

	asio::async_write(socket, asio::buffer(pkg), handler);
	//socket.async_write_some(asio::buffer(pkg), handler);
	if(io_service.stopped()){
		io_service.restart();
		boost::thread(boost::bind(&asio::io_service::run, &io_service));
	}
	pkgs_sent ++;
	//std::cout << " S:" << pkgs_sent << endl;
	//std::cout << " Sending: " << pkg << endl;

}

void IoClient::qread(){
	auto handler = boost::bind(&IoClient::handle_qread_content, this,
							   asio::placeholders::error(),
							   asio::placeholders::bytes_transferred());

	socket.async_read_some(asio::buffer(read_buffer, 1024), handler);

	if(io_service.stopped()){
		io_service.restart();
		boost::thread(boost::bind(&asio::io_service::run, &io_service));
	}
}

void IoClient::handle_qsent_content(const boost::system::error_code& error, std::size_t bytes_transferred){

	this->busy = false;

	if(error){
		//cout << "Error occurred! S[C" << this->id_client << "]: " << error << endl;
		this->connection_state = CONNECTION_STATE_DISCONNECTED;
		return;
	}
	if(!this->pkg_queue.empty()){
		this->qsend(this->pkg_queue.front().pkg, this->pkg_queue.front()._cb);
		this->pkg_queue.pop();
	}
	//pkgs_sent ++;
	//std::cout << " S:" << pkgs_sent << endl;
}


void IoClient::handle_qread_content(const boost::system::error_code& error, std::size_t bytes_transferred){

	if(error){
		cout << "Error occurred R[C" << this->id_client << "]: " << error << endl;
		this->connection_state = CONNECTION_STATE_DISCONNECTED;
		return;
	}

	//std::cout << "Data received[C" << this->id_client << "]: " ;

	std::string data((char*)read_buffer, bytes_transferred );
	//cout << data << endl;
	data = read_remainder + data;
	std::string pkg;

	while((pkg = extract_pkg(data)) != ""){
		pkgs_recv ++;
		//std::cout << " R:" << pkgs_recv << endl;

		boost::json::value pt;
/*
		if(this->wait_for_binary){
			this->wait_for_binary = false;
			//pt = this->wait_for_binary_pt;
			pt["action"] = "binary_transfer";
			pt["data"] = pkg;

			this->process_actions_fn(pt);

		} else {
*/
			//bool parse_ok = parse_json(pkg, pt);
			pt = boost::json::parse(pkg);

			if( pt.is_object() ){

				boost::json::object obj = pt.get_object();
				//cout << pkg << endl;
				/*
				if(pt["binary_transfer"]){//as bool
					this->wait_for_binary = true;
					this->wait_for_binary_pt = pt;
				}
				*/
				boost::json::value resp = obj["response"];
				if(!resp.is_null()){ //as string
					std::string response_name = resp.get_string().c_str();//.get<std::string>("response");

					for(std::size_t i = 0; i < cbs.size(); i++){
						if(cbs[i].name == response_name){ //si hay dos con el mismo nombre que pasa?
							cbs[i].cb(obj);

							auto t_end = std::chrono::high_resolution_clock::now();
							this->ping_ms = std::chrono::duration<double, std::milli>(t_end - cbs[i].c_s).count();
							//std::cout << "PING: " << this->ping_ms << "ms" << endl;

							cbs.erase(cbs.begin() + i);
							if(cbs.size() > 0){
								cout << "CM remaining: " << cbs.size() << endl;
							}


							break;
						}
					}

				}

				if(obj["action"].is_string() && this->process_actions_fn != nullptr){
					//std::cout << "ACCION!!!" << std::endl;
					this->process_actions_fn(obj);
				}
			}
			else{
				cout << "error parsing: " << pkg << endl;
			}
//		}
	}
	read_remainder = data;
	//cout << read_remainder << endl;
	qread();
}