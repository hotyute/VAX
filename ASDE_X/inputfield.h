#ifndef INPUTFIELD_H
#define INPUTFIELD_H

#include "gui.h"

class InputField : public ChildFrame {
public:
	virtual ~InputField();
	InputField(InterfaceFrame*, double, double);
	InputField(InterfaceFrame* frame, double x, double width, double padding_x, double, double, double);
public:
	bool centered = false, editable = true, numbers = false, caps = false, cursor_default = true;
	HFONT* font = nullptr;
	unsigned int* base = nullptr;
	ChatLine* line_ptr = nullptr;
	std::string pp_input;
	bool p_protected = false; // password protection
	std::string input;
	std::string cursor_input;
	int cursor_pos = 0, last_cursor_pos = 0;
	int inputTextDl = 0;
	int inputCursorDl = 0;
	int max_chars = 0;
	float rgb[3] = { 1.0f, 1.0f, 1.0f };

	int offset_x = 3, offset_y = 6;

	void updatePos(double x, double width, double y, double height);
	void doDrawing();
	void setFocus();
	void removeFocus();
	void doAction();
	void focusDrawing();
	int handleClick(ChildFrame* clicked, int x, int y);

	void pushInput(bool, char);
	void setCursorAtStart();
	void setCursorAtEnd();
	void setCursor();
	void setCursor(int pos);
	void clamp_cursor();
	void cursorLeft();
	void cursorRight();
	void removeCursor();
	bool popInput();
	void clearInput();
	void setInput(std::string, bool cursor);
	void setUneditable(std::string);
	void pass_characters(char* chars);
	bool can_type();
	void handleBox();
	void handleBox2();
	void updateLine();
	void updateInput(ChatLine* c);
	void handleEntry();
	void calcCursorPos(double x, double y);
};

#endif
