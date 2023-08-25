#ifndef INPUT_H
#define INPUT_H

#include <boost/json.hpp>
#include <string>

enum InputType {
	INPUT_TYPE_TEXT,
	INPUT_TYPE_SELECT,
	INPUT_TYPE_TOGGLE,
	INPUT_TYPE_MAX
};

class Input {
public:

	std::string label;

	//double x;

	//double y;

	InputType type;

	virtual void draw(float x, float y) = 0;

	virtual void focus() = 0;

	virtual void blur() = 0;

	virtual bool is_focused() = 0;

	virtual void set_from_json_value(boost::json::value& val) = 0;

	virtual boost::json::value get_json_value() = 0;

};

#endif
