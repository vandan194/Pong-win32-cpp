struct Button_State
{
	bool is_down, changed;
};
enum
{
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_SPACE,
	BUTTON_COUNT
};
struct Input
{
	Button_State buttons[BUTTON_COUNT];
};