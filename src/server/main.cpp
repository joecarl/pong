
#include <iostream>
#include <stdexcept>

#include "server.h"

using namespace std;

int main(int argc, char **argv){

	try{
		
		Server server(28090);

		server.run();
		
		return 0;

	} catch (std::exception &e) {

		cerr << "Runtime exception: " << e.what() << endl;
		return -1;
	
	}

}
