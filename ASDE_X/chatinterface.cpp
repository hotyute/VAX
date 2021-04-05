#include "chatinterface.h"
#include "projection.h"

std::vector<BasicInterface*> interfaces;

BasicInterface::BasicInterface() {
	BasicInterface::render = false;
}

BasicInterface::BasicInterface(double posX, double width, double width_padding, double posY, double height, double height_padding, float r, float g, float b, float transparency, bool absolute, bool wire_mode) {
	BasicInterface::width = width;
	BasicInterface::width_padding = width_padding;
	BasicInterface::height = height;
	BasicInterface::height_padding = height_padding;
	BasicInterface::transparency = transparency;
	color[0] = r;
	color[1] = g;
	color[2] = b;
	BasicInterface::absolute = absolute;
	BasicInterface::wire_mode = wire_mode;
	BasicInterface::posX = posX;
	BasicInterface::posY = posY;
	BasicInterface::render = true;
	BasicInterface::bounds = false;
}

void BasicInterface::createMutex() {
	BasicInterface::mutex = CreateMutex(NULL, FALSE, L"CI Mutex");
}

double BasicInterface::getWidth() {
	return BasicInterface::width;
}

void BasicInterface::setActualWidth(double actual) {
	BasicInterface::actual_width = actual;
}

double BasicInterface::getActualWidth() {
	return BasicInterface::actual_width;
}

double BasicInterface::getWidthPad() {
	return BasicInterface::width_padding;
}

void BasicInterface::setActualHeight(double actual) {
	BasicInterface::actual_height = actual;
}

double BasicInterface::getActualHeight() {
	return BasicInterface::actual_height;
}

double BasicInterface::getHeight() {
	return BasicInterface::height;
}

double BasicInterface::getHeightPad() {
	return BasicInterface::height_padding;
}

float BasicInterface::getTransparency() {
	return BasicInterface::transparency;
}

void BasicInterface::updateXs(int x, int x2) {
	BasicInterface::x[0] = x;
	BasicInterface::x[1] = x2;
}

void BasicInterface::updateYs(int y, int y2) {
	BasicInterface::y[0] = y;
	BasicInterface::y[1] = y2;
}

void BasicInterface::clearPoints() {
	BasicInterface::x[0] = -1;
	BasicInterface::x[1] = -1;
	BasicInterface::y[0] = -1;
	BasicInterface::y[1] = -1;
}

int BasicInterface::getStartX() {
	return BasicInterface::x[0];
}

int BasicInterface::getStartY() {
	return BasicInterface::y[0];
}

int BasicInterface::getEndX() {
	return BasicInterface::x[1];
}

int BasicInterface::getEndY() {
	return BasicInterface::y[1];
}

void BasicInterface::getColor(float *dest) {
	dest[0] = BasicInterface::color[0];
	dest[1] = BasicInterface::color[1];
	dest[2] = BasicInterface::color[2];
}

bool BasicInterface::isWireMode() {
	return BasicInterface::wire_mode;
}

bool BasicInterface::isAbsolute() {
	return BasicInterface::absolute; 
}

void BasicInterface::updateCoordinates() {
	double xs[3];
	double ys[3];
	BasicInterface::updateCoordinates(xs, ys);
	double start_x = xs[0], end_x = xs[1];
	double start_y = ys[0], end_y = ys[1];
	BasicInterface::updateXs(start_x, end_x);
	BasicInterface::updateYs(start_y, end_y);
	BasicInterface::actual_width = xs[2];
	BasicInterface::actual_height = ys[2];
}

void BasicInterface::updateCoordinates(double* xs, double* ys) {
	//x[0] x start, x[1] x end
	//same with y
	int w_width = CLIENT_WIDTH, w_height = CLIENT_HEIGHT;

	double f_width = BasicInterface::width, f_height = BasicInterface::height;
	if (!absolute) {		
		if (BasicInterface::width <= 1.0) {// percentage
			f_width = (BasicInterface::width * w_width);
			xs[0] = ((w_width - f_width) + width_padding);
			xs[1] = (w_width + width_padding);
		} else {
			f_width = BasicInterface::width;
			xs[0] = ((w_width - f_width) + width_padding);
			xs[1] = (w_width + width_padding);
		}
		if (BasicInterface::height <= 1.0) {//percentage
			f_height = (BasicInterface::height * w_height);
			ys[0] = ((w_height - f_height) + height_padding);
			ys[1] = (w_height + height_padding);
		} else {
			f_height = BasicInterface::height;
			ys[0] = ((w_height - f_height) + height_padding);
			ys[1] = (w_height + height_padding);
		}
	} else {
		xs[0] = (BasicInterface::posX + BasicInterface::width_padding);
		if (BasicInterface::width <= 1.0) {
			f_width = (BasicInterface::width * w_width);
			xs[1] =  ((BasicInterface::posX + f_width) + BasicInterface::width_padding);
		} else {
			xs[1] = ((BasicInterface::posX + BasicInterface::width) + BasicInterface::width_padding);
		}
		ys[0] = (BasicInterface::posY + BasicInterface::height_padding);
		if (BasicInterface::height <= 1.0) {
			f_height = (BasicInterface::height * w_height);
			ys[1] =  ((BasicInterface::posY + f_height) + BasicInterface::height_padding);
		} else {
			ys[1] = ((BasicInterface::posY + BasicInterface::height) + BasicInterface::height_padding);
		}
	}
	xs[2] = f_width;
	ys[2] = f_height;
}

void BasicInterface::setPosX(int x)
{
	BasicInterface::posX = x;
}

int BasicInterface::getPosX() {
	return BasicInterface::posX;
}

void BasicInterface::setPosY(int y)
{
	BasicInterface::posY = y;
}

int BasicInterface::getPosY() {
	return BasicInterface::posY;
}

void BasicInterface::setRender(bool draw) {
	BasicInterface::render = draw;
}

bool BasicInterface::isRender() {
	return BasicInterface::render;
}

void BasicInterface::setBounds(bool draw) {
	BasicInterface::bounds = draw;
}

bool BasicInterface::isBounds() {
	return BasicInterface::bounds;
}