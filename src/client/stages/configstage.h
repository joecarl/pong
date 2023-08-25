#ifndef CONFIGSTAGE_H
#define CONFIGSTAGE_H

#include "../hengine.h"
#include "../ui/textinput.h"

#include <allegro5/allegro.h>
#include <vector>


typedef struct {

	std::string key;

	std::string label;

	Input* input;

} ConfigParam;

//-----------------------------------------------------------------------------

class ConfigStage: public Stage {

	std::vector<ConfigParam> config_params;

	uint8_t inp_index = 0;

	bool welcome_view;

	void draw_welcome_view();

	void draw_config_view();

	bool saved_modified_inputs();

	bool saved;

public:

	ConfigStage(HGameEngine* _engine);

	void on_enter_stage();

	void on_event(ALLEGRO_EVENT event);

	void draw();

};

#endif