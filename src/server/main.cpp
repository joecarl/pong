
#include <iostream>
#include <stdexcept>

using namespace std;

int main(int argc, char **argv){

	try{
		
		
		
		return 0;

	} catch (std::exception &e) {

		cerr << "Runtime exception: " << e.what() << endl;
		return -1;
	
	}

}
