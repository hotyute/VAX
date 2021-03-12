#include "chatinterface.h"
#include "projection.h"

std::vector<ChatInterface*> interfaces;

ChatInterface::ChatInterface() {
	ChatInterface::render = false;
}

ChatInterface::ChatInterface(double posX, double width, double width_padding, double posY, double height, double height_padding, float r, float g, float b, float transparency, bool absolute, bool wire_mode) {
	ChatInterface::width = width;
	ChatInterface::width_padding = width_padding;
	ChatInterface::height = height;
	ChatInterface::height_padding = height_padding;
	ChatInterface::transparency = transparency;
	color[0] = r;
	color[1] = g;
	color[2] = b;
	ChatInterface::absolute = absolute;
	ChatInterface::wire_mode = wire_mode;
	ChatInterface::posX = posX;
	ChatInterface::posY = posY;
	ChatInterface::render = true;
}

void ChatInterface::createMutex() {
	ChatInterface::mutex = CreateMutex(NULL, FALSE, L"CI Mutex");
}

double ChatInterface::getWidth() {
	return ChatInterface::width;
}

void ChatInterface::setActualWidth(double actual) {
	ChatInterface::actual_width = actual;
}

double ChatInterface::getActualWidth() {
	return ChatInterface::actual_width;
}

double ChatInterface::getWidthPad() {
	return ChatInterface::width_padding;
}

void ChatInterface::setActualHeight(double actual) {
	ChatInterface::actual_height = actual;
}

double ChatInterface::getActualHeight() {
	return ChatInterface::actual_height;
}

double ChatInterface::getHeight() {
	return ChatInterface::height;
}

double ChatInterface::getHeightPad() {
	return ChatInterface::height_padding;
}

float ChatInterface::getTransparency() {
	return ChatInterface::transparency;
}

void ChatInterface::updateXs(int x, int y) {
	ChatInterface::x[0] = x;
	ChatInterface::x[1] = y;
}

void ChatInterface::updateYs(int x, int y) {
	ChatInterface::y[0] = x;
	ChatInterface::y[1] = y;
}

void ChatInterface::clearPoints() {
	ChatInterface::x[0] = -1;
	ChatInterface::x[1] = -1;
	ChatInterface::y[0] = -1;
	ChatInterface::y[1] = -1;
}

int ChatInterface::getStartX() {
	return ChatInterface::x[0];
}

int ChatInterface::getStartY() {
	return ChatInterface::y[0];
}

int ChatInterface::getEndX() {
	return ChatInterface::x[1];
}

int ChatInterface::getEndY() {
	return ChatInterface::y[1];
}

void ChatInterface::getColor(float *dest) {
	dest[0] = ChatInterface::color[0];
	dest[1] = ChatInterface::color[1];
	dest[2] = ChatInterface::color[2];
}

bool ChatInterface::isWireMode() {
	return ChatInterface::wire_mode;
}

bool ChatInterface::isAbsolute() {
	return ChatInterface::absolute; 
}

void ChatInterface::updateCoordinates() {
	double xs[3];
	double ys[3];
	ChatInterface::updateCoordinates(xs, ys);
	double start_x = xs[0], end_x = xs[1];
	double start_y = ys[0], end_y = ys[1];
	ChatInterface::updateXs(start_x, end_x);
	ChatInterface::updateYs(start_y, end_y);
	ChatInterface::actual_width = xs[2];
	ChatInterface::actual_height = ys[2];
}

void ChatInterface::updateCoordinates(double* xs, double* ys) {
	//x[0] x start, x[1] x end
	//same with y
	int w_width = CLIENT_WIDTH;
	if (w_width < FIXED_CLIENT_WIDTH) {
		w_width = FIXED_CLIENT_WIDTH;
	}
	int w_height = HEIGHT;
	if (w_height < FIXED_CLIENT_HEIGHT) {
		w_height = FIXED_CLIENT_HEIGHT;
	}
	double f_width = ChatInterface::width, f_height = ChatInterface::height;
	if (!absolute) {		
		if (ChatInterface::width <= 1.0) {// percentage
			f_width = (ChatInterface::width * w_width);
			xs[0] = ((w_width - f_width) + width_padding);
			xs[1] = (w_width + width_padding);
		} else {
			f_width = ChatInterface::width;
			xs[0] = ((w_width - f_width) + width_padding);
			xs[1] = (w_width + width_padding);
		}
		if (ChatInterface::height <= 1.0) {//percentage
			f_height = (ChatInterface::height * w_height);
			ys[0] = ((w_height - f_height) + height_padding);
			ys[1] = (w_height + height_padding);
		} else {
			f_height = ChatInterface::height;
			ys[0] = ((w_height - f_height) + height_padding);
			ys[1] = (w_height + height_padding);
		}
	} else {
		xs[0] = (ChatInterface::posX + ChatInterface::width_padding);
		if (ChatInterface::width <= 1.0) {
			f_width = (ChatInterface::width * w_width);
			xs[1] =  ((ChatInterface::posX + f_width) + ChatInterface::width_padding);
		} else {
			xs[1] = ((ChatInterface::posX + ChatInterface::width) + ChatInterface::width_padding);
		}
		ys[0] = (ChatInterface::posY + ChatInterface::height_padding);
		if (ChatInterface::height <= 1.0) {
			f_height = (ChatInterface::height * w_height);
			ys[1] =  ((ChatInterface::posY + f_height) + ChatInterface::height_padding);
		} else {
			ys[1] = ((ChatInterface::posY + ChatInterface::height) + ChatInterface::height_padding);
		}
	}
	xs[2] = f_width;
	ys[2] = f_height;
}

int ChatInterface::getPosX() {
	return ChatInterface::posX;
}

int ChatInterface::getPosY() {
	return ChatInterface::posY;
}

void ChatInterface::setRender(bool draw) {
	ChatInterface::render = draw;
}

bool ChatInterface::isRender() {
	return ChatInterface::render;
}

void ChatInterface::setBounds(bool draw) {
	ChatInterface::bounds = draw;
}

bool ChatInterface::isBounds() {
	return ChatInterface::bounds;
}