#ifndef INPUT_H
#define INPUT_H

enum InputType {
	INPUT_TYPE_TEXT,
	INPUT_TYPE_SELECT,
	INPUT_TYPE_TOGGLE,
	INPUT_TYPE_MAX
};

class Input {
public:
	InputType type;
};

#endif
