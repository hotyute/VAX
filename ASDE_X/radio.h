#ifndef RADIO_H
#define RADIO_H

#include "gui.h"

class Radio : public ChildFrame {
public:
	Radio(InterfaceFrame* border, double x, double width, double y, double height);
public:
	bool checked = true;

	void updatePos(double x, double width, double y, double height);
	void doDrawing();
	void setFocus();
	void removeFocus();
	void doAction();
	void focusDrawing();
	int handleClick(ChildFrame* clicked, int x, int y);
};

#endif
