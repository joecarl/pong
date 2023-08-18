
#include <iostream>
#include <stdexcept>

#include "hengine.h"
#include "../utils.h"
#include "../appinfo.h"


using namespace std;

int main(int argc, char **argv) {

	try {

		if (argc == 2 && strcmp(argv[1], "--version") == 0) {
			cout << APP_VERSION;
			return 0;
		}
		/*
		string v = trim(exec("updater checkout"));

		cout << "Version checkout: " << v << "(" << v.length() << ")" << endl;

		if (v.length() > 0 && v != APP_VERSION) {

			//system("updater &");
			//here should open updater process
			return 0;
		}
		*/
		HGameEngine game_engine;
		
		game_engine.set_stage(MENU);

		game_engine.run();
		
		return 0;

	} catch (std::exception &e) {

		cerr << "Runtime exception: " << e.what() << endl;
		return -1;
	
	}

}
