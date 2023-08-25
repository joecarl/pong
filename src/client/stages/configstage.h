#ifndef CONFIGSTAGE_H
#define CONFIGSTAGE_H


#include "../hengine.h"
#include "../ui/textinput.h"
#include <allegro5/allegro.h>

//-----------------------------------------------------------------------------

class ConfigStage: public Stage {

	TextInput* input;

public:

	ConfigStage(HGameEngine* _engine);

	void on_enter_stage();

	void on_event(ALLEGRO_EVENT event);

	void draw();

};

#endif