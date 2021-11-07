#include "radio.h"

#include "renderer.h"
#include "tools.h"

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
	Radio::border->setPosX((int)x), Radio::border->setPosY((int)y);
	Radio::border->setWidth(width), Radio::border->setHeight(height);
	Radio::border->updateCoordinates();
}

void Radio::doDrawing()
{
	if (checked)
	{
		BasicInterface* inter11 = Radio::border;

		double offset = 2;

		double x_half = ((inter11->getEndX() - inter11->getStartX()) / 2) - offset;
		double y_half = ((inter11->getEndY() - inter11->getStartY()) / 2) - offset;

		int startX = inter11->getStartX(), startY = inter11->getStartY();
		int endX = inter11->getEndX(), endY = inter11->getEndY();

		glColor4f(0.0f, 0.5f, 0.0f, 1.0f);
		int line_width = 2;
		startX += line_width, startY += line_width;
		endX -= line_width, endY -= line_width;
		DrawVarLine(startX, endY - y_half, endX - x_half, startY, line_width, line_width);
		DrawVarLine(startX + x_half, startY, endX, endY, line_width, line_width);
	}
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
	BasicInterface* inter2 = Radio::border;
	double vertx[4] = { inter2->getStartX(), inter2->getStartX(), inter2->getEndX(), inter2->getEndX() };
	double verty[4] = { inter2->getStartY(), inter2->getEndY(), inter2->getEndY(), inter2->getStartY() };
	bool click = pnpoly(4, vertx, verty, x, y);
	if (click)
	{
		checked = !checked;
		renderDrawings = true;
		return 1;
	}
	return 0;
}
