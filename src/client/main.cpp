
#include "pongclient.h"
#include "../appinfo.h"
#include <dp/utils.h>
#include <iostream>
#include <stdexcept>


int main(int argc, char **argv) {

	try {
		// TODO: incluir params processing en el cliente
		if (argc == 2 && strcmp(argv[1], "--version") == 0) {
			std::cout << APP_VERSION;
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
		PongClient game_client;

		auto& cfg = game_client.get_cfg().json();

		int stage = cfg.contains("playerName") ? MENU : CONF;

		game_client.set_stage(stage);
		
		game_client.run();
		
		return 0;

	} catch (std::exception &e) {

		std::cerr << "Runtime exception: " << e.what() << std::endl;
		return -1;
	
	}

}
