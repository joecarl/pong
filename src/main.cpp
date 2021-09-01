
#include <iostream>
#include <stdexcept>

#include "hengine.h"


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


/*
void timer_task(int *ms){
	while (1) {
		//usleep(1000);
		boost::this_thread::sleep(boost::posix_time::milliseconds(1));
		(*ms)++;
	}
}

int milliseconds = 0;
boost::thread th_tim(timer_task, &milliseconds);
*/
