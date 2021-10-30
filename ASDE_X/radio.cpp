#include "radio.h"

Radio::Radio(InterfaceFrame* frame, double x, double width, double y, double height)
{
	Radio::frame = frame;
	Radio::focus = false;
	Radio::type = CHILD_TYPE::RADIO;
	BasicInterface* fieldBounds = new BasicInterface(x, width, 0.0, y, height, 0.0, 1.0f, 1.0f, 1.0f, 0.8, true, true);
	fieldBounds->setBounds(true);
	fieldBounds->updateCoordinates();
	Radio::border = fieldBounds;
	Radio::child_interfaces.push_back(fieldBounds);
}

void Radio::updatePos(double x, double width, double y, double height)
{
	Radio::border->setPosX(x), Radio::border->setPosY(y);
	Radio::border->setWidth(width), Radio::border->setHeight(height);
	Radio::border->updateCoordinates();
}

void Radio::doDrawing()
{
}

void Radio::setFocus()
{
}

void Radio::removeFocus()
{
}

void Radio::doAction()
{
}

void Radio::focusDrawing()
{
}

int Radio::handleClick(ChildFrame* clicked, int x, int y)
{
	return 0;
}
