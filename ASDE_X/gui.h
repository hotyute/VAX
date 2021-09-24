#ifndef GUI_H
#define GUI_H

#include <iostream>
#include <vector>

#include "chatinterface.h"
#include "projection.h"

#define NUM_SUB_INTERFACES 2

enum class CHAT_TYPE { MAIN, ERRORS, SYSTEM, ATC, SUP, SUP_POS };
enum class CHILD_TYPE {
	LABEL_D,
	INPUT_FIELD,
	CLOSE_BUTTON,
	CLICK_BUTTON,
	COMBO_BOX,
	DISPLAY_BOX
};

class InterfaceFrame;
class ChildFrame;
class ChatLine;

class InterfaceFrame {
private:
public:
	int id = -1;
	unsigned int interfaceDl = 0, drawingDl = 0, focusDl = 0;
	POINT *s_pt = nullptr, *cur_pt = nullptr, *end_pt = nullptr;
	BasicInterface* border = nullptr;
	BasicInterface* move_bound = nullptr;
	bool render, renderAllInputText, renderAllLabels, multi_open = true, pannable = false;
	int index = -1;
	std::string title;
	std::vector<BasicInterface*> interfaces;
	std::vector<ChildFrame*> children;
	~InterfaceFrame();
	InterfaceFrame(int);
	InterfaceFrame(int, double, double);
	InterfaceFrame(int, double, double, double, double);
	void Pane1(double, double, double, double);
	void UpdatePane1(double, double, double, double);
	void doOpen(bool multi_open, bool pannable);
	void doClose();
	void doReplace();
	void doDefinition();
	void doInsert();
	void move(int dx, int dy);
	bool withinClient();
};

class ChildFrame {
protected:
	InterfaceFrame* frame;
public:
	virtual ~ChildFrame();
	std::vector<BasicInterface*> child_interfaces;
	BasicInterface *border = nullptr;
	CHILD_TYPE type;
	int index;
	bool focus, show_border = true;
	virtual void updatePos(double x, double width, double y, double height) = 0;
	virtual void doDrawing() = 0;
	virtual void setFocus() = 0;
	virtual void removeFocus() = 0;
	virtual void doAction() = 0;
	virtual void focusDrawing() = 0;
	virtual int handleClick(ChildFrame* clicked, int x, int y) = 0;
	InterfaceFrame* getFrame() { return this->frame; }
};

class ChatLine {
private:
	std::string line;
	CHAT_TYPE type;
	int _x = -1, _y = -1, _s_x = -1, _s_y = -1, _p_x = -1, _p_y = -1;
public:
	ChatLine* split = nullptr;
	ChatLine(std::string, CHAT_TYPE);
	~ChatLine();
	void setType(CHAT_TYPE type);
	CHAT_TYPE getType();
	void setText(std::string text);
	std::string getText();
	void playChatSound();
	void set_p(int x, int y, int t_x, int t_y, int e_x, int e_y) 
	{
		this->_x = x; this->_y = y; this->_s_x = t_x; this->_s_y = t_y; this->_p_x = e_x; this->_p_y = e_y;
	}
	int get_x() { return _x; }
	int get_y() { return _y; }
	int size_x() { return _p_x; }
	int size_y() { return _p_y; }
	bool in_bounds(int x, int y);
	bool in_bounds_text(int x, int y);
	void reset_p() { this->_x = -1; this->_y = -1; this->_s_x = -1; this->_s_y = -1; }
	bool has_p() { this->_x != -1 && this->_y != -1; }
};

class InputField : public ChildFrame {
public:
	virtual ~InputField();
	InputField(InterfaceFrame*, double, double);
	InputField(InterfaceFrame*, double, double, double, double, double, double);
public:
	bool centered, editable;
	ChatLine* line_ptr = nullptr;
	std::string pp_input;
	bool p_protected; // password protection
	std::string input;
	std::string cursor_input;
	int cursor_pos = 0, last_cursor_pos = 0;
	int inputTextDl = 0;
	int inputCursorDl = 0;
	int max_chars = 0;

	int offset_x = 3, offset_y = 6;

	void updatePos(double x, double width, double y, double height);
	void doDrawing();
	void setFocus();
	void removeFocus();
	void doAction();
	void focusDrawing();
	int handleClick(ChildFrame* clicked, int x, int y);

	void pushInput(bool, char);
	void setCursorAtEnd();
	void setCursor();
	void cursorLeft();
	void cursorRight();
	void removeCursor();
	void popInput();
	void clearInput();
	void setInput(std::string);
	void setUneditable(std::string);
	void pass_characters(char* chars);
	bool can_type();
	void handleBox();
	void handleEntry();
};

class CloseButton : public ChildFrame {
public:
	CloseButton(InterfaceFrame*, double, double);
public:
	void updatePos(double x, double width, double y, double height);
	void doDrawing();
	void setFocus();
	void removeFocus();
	void doAction();
	void focusDrawing();
	int handleClick(ChildFrame* clicked, int x, int y);
};

class ClickButton : public ChildFrame {
public:
	ClickButton(InterfaceFrame*, std::string, double x, double width, double y, double height);
public:
	int border_pix = 4;
	std::string text;

	void updatePos(double x, double width, double y, double height);
	void doDrawing();
	void setFocus();
	void removeFocus();
	void doAction();
	void focusDrawing();
	int handleClick(ChildFrame* clicked, int x, int y);
};

class ComboBox : public ChildFrame {
public:
	ComboBox(InterfaceFrame*, std::vector<std::string>, double, double, double, double, double, double);
public:
	size_t pos;
	std::vector<std::string> options;
	std::vector<SIZE> extents;
	SIZE largestExtent;

	void updatePos(double x, double width, double y, double height);
	void doDrawing();
	void setFocus();
	void removeFocus();
	void doAction();
	void focusDrawing();
	int handleClick(ChildFrame* clicked, int x, int y);
};

class DisplayBox : public ChildFrame {
private:
	int noncp = 2;
public:
	DisplayBox(InterfaceFrame*, std::vector<ChatLine*>, int, double, double, double, double, double, double, bool);
	~DisplayBox();
public:
	int numBlocks;
	int read_index = 0, max_history = 100;
	bool centered, editable = false, prune_top = false;
	std::vector<ChatLine*> chat_lines;
	std::vector<ChatLine*> displayed_lines;

	void updatePos(double x, double width, double y, double height);
	void doDrawing();
	void setFocus();
	void removeFocus();
	void doAction();
	void focusDrawing();
	int handleClick(ChildFrame* clicked, int x, int y);

	void resetReaderIdx();
	void resetReaderIdxTop();
	void doActionUp();
	void doActionDown();
	void addLine(std::string, CHAT_TYPE type);
	void addLineTop(std::string, CHAT_TYPE type);
	void addLine(ChatLine* c);
	void addLineTop(ChatLine* c);
	void removeLine(ChatLine* c);
	void SetChatTextColour(CHAT_TYPE t);
	void clearLines();
	void prepare();
	void editText(ChatLine* line, int x, int y);

	bool click_arrow_bottom(int x, int y, int arrow_bounds, int arrow_offset);
	bool click_arrow_top(int x, int y, int arrow_bounds, int arrow_offset);

};

class Label : public ChildFrame {
public:
	Label(InterfaceFrame*, std::string, double, double);
	Label(InterfaceFrame*, std::string, double, double, double, double, double, double);
public:
	int centered;
	std::string input;
	int labelTextDl = 0;

	void updatePos(double x, double width, double y, double height);
	void doDrawing();
	void setFocus();
	void removeFocus();
	void doAction();
	void focusDrawing();
	int handleClick(ChildFrame* clicked, int x, int y);
};

extern std::vector<InterfaceFrame*> frames_def;
extern std::vector<InterfaceFrame*> rendered_frames;
extern ChildFrame *focusChild, *lastFocus;
extern std::vector<InterfaceFrame*> deleteInterfaces, updateInterfaces;
extern bool updateLastFocus;
extern InterfaceFrame* _openedframe;



#endif
