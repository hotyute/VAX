#include <Windows.h>
#include <iostream>
#include <vector>

class ChatInterface {
private:
	HANDLE mutex;
	int x[2];
	int y[2];
	double actual_width, width;
	double width_padding;
	double actual_height, height;
	double height_padding;
	float transparency;
	float color[3];
	bool wire_mode;
	bool absolute;
	int posX, posY;
	bool render, bounds;
public:
	ChatInterface();
	ChatInterface(double posX, double width, double width_padding, double posY, double height, double height_padding, float r, float g, float b, float transparency, bool absolute, bool wire_mode);
	void createMutex();
	void setActualWidth(double);
	double getActualWidth();
	double getWidth();
	double getWidthPad();
	void setActualHeight(double);
	double getActualHeight();
	double getHeight();
	double getHeightPad();
	float getTransparency();
	void updateXs(int, int);
	void updateYs(int, int);
	void clearPoints();
	int getStartX();
	int getStartY();
	int getEndX();
	int getEndY();
	void getColor(float*);
	bool isWireMode();
	bool isAbsolute();
	void updateCoordinates();
	void updateCoordinates(double*, double*);
	int getPosX();
	int getPosY();
	bool isRender();
	void setRender(bool);
	bool isBounds();
	void setBounds(bool);
};