#ifndef GUI_H
#define GUI_H

#include <iostream>
#include <vector>

#include "chatinterface.h"
#include "projection.h"

#define LABEL_D 62442
#define INPUT_FIELD 62443
#define CLOSE_BUTTON 62444
#define CLICK_BUTTON 62445
#define COMBO_BOX 62446
#define DISPLAY_BOX 62447

#define NUM_SUB_INTERFACES 2

enum class CHAT_TYPE { MAIN, ERRORS, SYSTEM, ATC };

class InterfaceFrame;
class ChildFrame;
class ChatLine;

class InterfaceFrame {
private:
public:
	unsigned int interfaceDl = 0, drawingDl = 0, focusDl = 0;
	POINT *s_pt = nullptr, *cur_pt = nullptr, *end_pt = nullptr;
	BasicInterface* move_bound = nullptr;
	bool render, renderAllInputText, renderAllLabels, multi_open = true, pannable = false;
	int index;
	std::string title;
	std::vector<BasicInterface*> interfaces;
	std::vector<ChildFrame*> children;
	InterfaceFrame(int);
	InterfaceFrame(int, double, double);
	InterfaceFrame(int, double, double, double, double);
	void Pane1(double, double, double, double);
	void doOpen(bool multi_open, bool pannable);
	void doClose();
};

class ChildFrame {
public:
	std::vector<BasicInterface*> child_interfaces;
	BasicInterface *border;
	int type;
	int index;
	bool focus;
	virtual void doDrawing() = 0;
	virtual void setFocus() = 0;
	virtual void removeFocus() = 0;
	virtual void doAction() = 0;
	virtual void focusDrawing() = 0;
};

class ChatLine {
private:
	CHAT_TYPE type;
	std::string line;
public:
	ChatLine(std::string, CHAT_TYPE);
	~ChatLine();
	void setType(CHAT_TYPE type);
	CHAT_TYPE getType();
	void setText(std::string text);
	std::string getText();
	void playChatSound();
};

class InputField : public ChildFrame {
private:
	InterfaceFrame *frame;
public:
	InputField(InterfaceFrame*, double, double);
	InputField(InterfaceFrame*, double, double, double, double, double, double);
public:
	bool centered, editable;
	std::string pp_input;
	bool p_protected; // password protection
	std::string input;
	int inputTextDl = 0;
	void doDrawing();
	void setFocus();
	void removeFocus();
	void doAction();
	void focusDrawing();
	void pushInput(bool, char);
	void popInput();
	void clearInput();
	void setInput(std::string);
	void setUneditable(std::string);
	void pass_characters(char* chars);
	bool can_type();
	InterfaceFrame* getFrame() { return this->frame; }
};

class CloseButton : public ChildFrame {
private:
	InterfaceFrame *frame;
public:
	CloseButton(InterfaceFrame*, double, double);
public:
	void doDrawing();
	void setFocus();
	void removeFocus();
	void doAction();
	void focusDrawing();
};

class ClickButton : public ChildFrame {
private:
	InterfaceFrame *frame;
public:
	ClickButton(InterfaceFrame*, std::string, double x, double width, double y, double height);
public:
	std::string text;
	void doDrawing();
	void setFocus();
	void removeFocus();
	void doAction();
	void focusDrawing();
};

class ComboBox : public ChildFrame {
private:
	InterfaceFrame *frame;
public:
	ComboBox(InterfaceFrame*, std::vector<std::string>, double, double, double, double, double, double);
public:
	size_t pos;
	std::vector<std::string> options;
	std::vector<SIZE> extents;
	SIZE largestExtent;
	void doDrawing();
	void setFocus();
	void removeFocus();
	void doAction();
	void focusDrawing();
};

class DisplayBox : public ChildFrame {
private:
	InterfaceFrame* frame;
public:
	DisplayBox(InterfaceFrame*, std::vector<ChatLine*>, int, double, double, double, double, double, double, bool);
public:
	int numBlocks;
	int read_index = 0, max_history = 100;
	bool centered;
	std::vector<ChatLine*> chat_lines;
	std::vector<ChatLine*> chat_line_history;
	void doDrawing();
	void setFocus();
	void removeFocus();
	void doAction();
	void focusDrawing();
	void display_pos();
	void resetReaderIdx();
	void doActionUp();
	void doActionDown();
	void addLine(std::string, CHAT_TYPE type);
	void addLine(ChatLine* c);


};

class Label : public ChildFrame {
private:
	InterfaceFrame* frame;
public:
	Label(InterfaceFrame*, std::string, double, double);
	Label(InterfaceFrame*, std::string, double, double, double, double, double, double);
public:
	int centered;
	std::string input;
	int labelTextDl = 0;
	void doDrawing();
	void setFocus();
	void removeFocus();
	void doAction();
	void focusDrawing();
};



extern std::vector<InterfaceFrame*> frames;
extern ChildFrame *focusChild, *lastFocus;
extern std::vector<InterfaceFrame*> deleteInterfaces, updateInterfaces;
extern bool updateLastFocus;
extern InterfaceFrame* _openedframe;



#endif
