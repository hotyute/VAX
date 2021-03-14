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

enum class CHAT_TYPE { MAIN, ERRORS, SYSTEM };

class InterfaceFrame;
class ChildFrame;
class ChatLine;

class InterfaceFrame {
private:
public:
	bool render, renderAllInputText, multi_open = true;
	int index;
	std::string title;
	std::vector<ChatInterface*> interfaces;
	std::vector<ChildFrame*> children;
	InterfaceFrame(int);
	InterfaceFrame(int, double, double);
	InterfaceFrame(int, double, double, double, double);
	void Pane1(double, double, double, double);
	void doOpen(int, bool multi_open);
	void doClose();
};

class ChildFrame {
public:
	std::vector<ChatInterface*> child_interfaces;
	ChatInterface *border;
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
	int inputTextDl;
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
	ClickButton(InterfaceFrame*, std::string, double, double, double, double);
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
	bool centered;
	std::vector<ChatLine*> list;
	void doDrawing();
	void setFocus();
	void removeFocus();
	void doAction();
	void focusDrawing();
	void addLine(std::string, CHAT_TYPE type);
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
	int labelTextDl;
	void doDrawing();
	void setFocus();
	void removeFocus();
	void doAction();
	void focusDrawing();
};



extern std::vector<InterfaceFrame*> frames;
extern ChildFrame *focusChild, *lastFocus;
extern std::vector<InterfaceFrame*> deleteInterfaces;
extern bool updateLastFocus;
extern InterfaceFrame* _openedframe;

#endif
