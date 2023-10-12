#ifndef STAGES_H
#define STAGES_H

#include <dp/client/stage.h>
#include <dp/client/ui/retrolines.h>
#include <allegro5/allegro.h>


class MainMenuStage: public dp::client::Stage {

	int easteregg = 0;

	ALLEGRO_BITMAP* logo;

	dp::client::ui::RetroLines retro_logo;

public:

	MainMenuStage(dp::client::BaseClient* _engine);
	
	void on_enter_stage();

	void on_tick();

	void on_event(ALLEGRO_EVENT event);

	void draw();

};


//-----------------------------------------------------------------------------

class GameOverStage: public dp::client::Stage {

public:

	using Stage::Stage;

	void on_enter_stage();

	void on_event(ALLEGRO_EVENT event);

	void draw();

};


#endif
