#ifndef TUTORIALSTAGE_H
#define TUTORIALSTAGE_H

#include <dp/client/stage.h>

#define THUMB_PRESS RES_DIR"/thumb_press.png"
#define THUMB_RELEASE RES_DIR"/thumb_release.png"

class TutorialStage: public dp::client::Stage {

	ALLEGRO_BITMAP* thumb_press;

	ALLEGRO_BITMAP* thumb_release;
	
	void draw_mode_one();
	
	void draw_mode_two();

public:

	using Stage::Stage;

	void on_enter_stage();

	void on_event(ALLEGRO_EVENT event);

	void draw();

};

#endif
