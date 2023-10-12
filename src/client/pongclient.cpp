
#include "pongclient.h"
#include "stages/stages.h"
#include "stages/gamestage.h"
#include "stages/onlinestages.h"
#include "stages/tutorialstage.h"
#include "stages/configstage.h"


void GameHandler::setup(int _play_mode, int _control_mode) {

	this->play_mode = _play_mode;
	this->control_mode = _control_mode;
	
}

void GameHandler::make_new_pong_game(int_fast32_t seed) {

	this->cleanup();
	pong_game = new PongGame(seed);
	pong_game->control_mode = this->control_mode;

}

void GameHandler::cleanup() {

	delete pong_game;
	pong_game = nullptr;

}

GameHandler::~GameHandler() {

	this->cleanup();

}

void GameHandler::set_player_name(uint8_t player_idx, const std::string& name) {

	if (player_idx >= 2) {
		return;
	}

	this->players_names[player_idx] = name;

}

std::string GameHandler::get_player_name(uint8_t player_idx) {

	if (player_idx >= 2) {
		return "";
	}

	std::string pname = this->players_names[player_idx];
	return pname == "" ? "PLAYER " + std::to_string(player_idx + 1) : pname;

}

/**
 * Retrieves the game CONTROL_* based on control_mode, keycode and player_idx
 */
int GameHandler::get_control(int k_code, uint8_t player_idx) {

	if (this->pong_game->control_mode == CONTROLMODE_TWO_PLAYERS && this->play_mode == PLAYMODE_LOCAL) {

		if (player_idx == 1) {

			if (k_code == ALLEGRO_KEY_UP || k_code == ALLEGRO_KEY_I) return CONTROL_MOVE_UP;
			else if (k_code == ALLEGRO_KEY_DOWN || k_code == ALLEGRO_KEY_K) return CONTROL_MOVE_DOWN;

		} else if (player_idx == 0) {

			if (k_code == ALLEGRO_KEY_W) return CONTROL_MOVE_UP;
			else if (k_code == ALLEGRO_KEY_S) return CONTROL_MOVE_DOWN;

		}

	} else {

		if (k_code == ALLEGRO_KEY_UP) return CONTROL_MOVE_UP;
		else if (k_code == ALLEGRO_KEY_DOWN) return CONTROL_MOVE_DOWN;

	}

	return CONTROL_NONE;

}


dp::client::Stage* PongClient::create_stage(uint16_t id) {

	switch (id) {
	case MENU:
		return new MainMenuStage(this);
	case GAME:
		return new GameStage(this);
	case OVER:
		return new GameOverStage(this);
	case CONN:
		return new ConnStage(this);
	case LOBBY:
		return new LobbyStage(this);
	case TUTO:
		return new TutorialStage(this);
	case CONF:
		return new ConfigStage(this);
	default:
		return nullptr;
	}
	
}