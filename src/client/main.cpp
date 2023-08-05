
#include <iostream>
#include <stdexcept>

#include "hengine.h"
#include "../utils.h"

#define CURR_VERSION "1.5"

using namespace std;

int main(int argc, char **argv) {

	try {

		if (argc == 2 && strcmp(argv[1], "--version") == 0) {
			cout << CURR_VERSION;
			return 0;
		}
		/*
		string v = trim(exec("updater checkout"));

		cout << "Version checkout: " << v << "(" << v.length() << ")" << endl;

		if (v.length() > 0 && v != CURR_VERSION) {

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
