#include "pongserver.h"

#include <iostream>
#include <stdexcept>

using std::cerr;
using std::endl;

int main(int argc, char **argv) {

	try {

		std::string port = "51009";

		for (int i = 0; i < argc; i++) {

			std::string arg = argv[i];

			if (arg == "-p" && i + 1 < argc) {
				port = argv[++i];
			}
			
		}
	
		PongServer server(stoi(port));
		server.run();
		
		return 0;

	} catch (std::exception &e) {

		cerr << "Runtime exception: " << e.what() << endl;
		return -1;
	
	}

}
