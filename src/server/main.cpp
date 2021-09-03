
#include <iostream>
#include <stdexcept>

#include "server.h"

using namespace std;

int main(int argc, char **argv){

	try{

		string port = "28090";

		for(int i = 0; i < argc; i++){

			string arg = argv[i];

			if(arg == "-p" && i + 1 < argc){
				port = argv[++i];
			}
			
		}

		
		Server server(stoi(port));

		server.run();
		
		return 0;

	} catch (std::exception &e) {

		cerr << "Runtime exception: " << e.what() << endl;
		return -1;
	
	}

}
