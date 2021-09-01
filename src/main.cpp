
#include <iostream>
#include <stdexcept>

#include "hengine.h"

using namespace std;

int main(int argc, char **argv){

	try{
		
		HGameEngine *gameEngine = new HGameEngine();
		
		gameEngine->setStage(MENU);

		gameEngine->run();
		
		return 0;

	} catch (std::exception &e) {

		cerr << "Runtime exception: " << e.what() << endl;
		return -1;
	
	}

}
