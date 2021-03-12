#ifndef RENDERER_H
#define RENDERER_H

#include <iostream>
#include <vector>

extern bool renderAircraft, renderSector, renderButtons, 
	renderLegend, renderAllCallsigns, renderInterfaces,
	renderInputText, renderConf, renderFocus, renderDrawings, what;

extern bool loadInterfaces;

extern bool resize;

extern int sectorDl, legendDl, buttonsDl, interfacesDl, confDl, aircraftDl, heavyDl, unkTarDl, focusDl, drawingDl;

extern unsigned int callSignBase, topButtonBase, confBase, legendBase, titleBase, labelBase;
extern HFONT callSignFont, topBtnFont, confFont, legendFont, titleFont, labelFont;

static const int scale[] = {4, 60};
static const int LOCKED_FPS = 30;
static const double NM_PER_DEG = 60.0;
static const double MIN_ZOOM = 0.00002;
static const double MAX_ZOOM = 30.0;

const static double BUTTON_PADDING = 0.05;
static int BUTTON_HEIGHT = 0;
const static double BUTTON_LAYOUT_HEIGHT = 0.1;

const static float day_background[] = {0.0f, 0.35686274509f, 0.49019607843f};
const static float runway_clr[] = {0.06274509803f, 0.06274509803f, 0.06274509803f};
const static float taxiway_clr[] = {0.16078431372f, 0.16078431372f, 0.16078431372f};
const static float apron_clr[] = {0.31764705882f, 0.31764705882f, 0.31764705882f};
const static float parking_clr[] = {0.38823529411f, 0.38823529411f, 0.38823529411f};
const static float regular_clr[] = {1.0f, 10.f, 10.f};
const static float heavy_clr[] = {0.99215686274f, 0.56862745098f, 0.15294117647f};
const static float button_clr_d[] = {0.22745098039f, 0.22745098039f, 0.22745098039f};
const static float button_clr_l[] = {0.32549019607f, 0.32549019607f, 0.32549019607f};
const static float button_bg[] = {0.44705882352f, 0.44705882352f, 0.44705882352f};
const static float button_text_clr[] = {0.80588235294f, 0.80588235294f, 0.80588235294f};
const static float callsign_clr[] = {0.83921568627f, 0.83921568627f, 0.31372549019f};
const static float conf_clr[] = {0.00392156862f, 0.72549019607f, 0.00392156862f};
const static float unknown_clr[] = {0.0f, 0.85490196078f, 0.81176470588f};

void InitOpenGL();
void ResizeGLScene();
void ResizeGLScene2(int, int, int, int);
void SetView(double, double);
void DrawGLScene();
DWORD WINAPI OpenGLThread(LPVOID lpParameter);
void SetPixelFormat(HDC);
int DrawSceneryData();
void RenderLegend();
void RenderButtons();
void RenderInterfaces();
void RenderConf();
void LoadInterfaces();
void GetOGLPos(int, int, double*);
void BuildFont(LPCWSTR, int, bool, unsigned int*, HFONT*);
void glPrint(const char*, unsigned int*, ...);
void deleteTess();
SIZE getTextExtent(std::string &s);
void DrawVarLine(float, float, float, float, float, float);

#endif
