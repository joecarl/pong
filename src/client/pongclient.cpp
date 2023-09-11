
#include "pongclient.h"
#include "stages/stages.h"
#include "stages/onlinestages.h"
#include "stages/tutorialstage.h"
#include "stages/configstage.h"


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