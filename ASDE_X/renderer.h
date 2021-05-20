#ifndef RENDERER_H
#define RENDERER_H

#include <iostream>
#include <vector>
#include <unordered_map>

#include "main.h"
#include <gl/GL.h>
#include <gl/GLU.h>

#include "projection.h"
#include "mirror.h"
#include "collision.h"

extern std::unordered_map<std::string, Mirror*> mirrors_storage;
extern std::vector<Mirror*> mirrors;

extern std::string heavy_text, regular_text;

extern bool renderAircraft, renderSector, renderSectorColours, renderButtons, renderLegend, renderInterfaces, renderInputTextFocus,
renderConf, renderDate, renderFocus, renderDrawings, queueDeleteInterface, renderDepartures, renderAllInputText;

extern bool updateFlags[NUM_FLAGS];
extern bool renderFlags[NUM_FLAGS];

extern bool resize;

extern int sectorDl, runwaysDl, taxiwaysDl, parkingDl, apronDl, holesDl, legendDl, buttonsDl, confDl, aircraftDl, heavyDl, unkTarDl;

extern unsigned int callSignBase, topButtonBase, confBase, legendBase, titleBase, labelBase, errorBase;
extern HFONT callSignFont, topBtnFont, confFont, legendFont, titleFont, labelFont, errorFont;

static const int scale[] = {4, 60};
static const int LOCKED_FPS = 60;
static const double NM_PER_DEG = 60.0;
static const double MIN_ZOOM = 0.00002;
static const double MAX_ZOOM = 30.0;

const static double BUTTON_PADDING = 0.05;
static double BUTTON_HEIGHT = 70;
static double BUTTON_WIDTH = 70;
const static double BUTTON_LAYOUT_HEIGHT = 0.1;

const static float day_background[] = {0.0f, 0.35686274509f, 0.49019607843f};
const static float nite_background[] = { 0.34509803921f, 0.34509803921f, 0.34509803921f };
const static float runway_clr[] = {0.06274509803f, 0.06274509803f, 0.06274509803f};
const static float taxiway_clr[] = {0.16078431372f, 0.16078431372f, 0.16078431372f};
const static float taxiway_nite_clr[] = { 0.06666666666f, 0.15294117647f, 0.31372549019f };
const static float apron_clr[] = {0.31764705882f, 0.31764705882f, 0.31764705882f};
const static float apron_nite_clr[] = { 0.07058823529f, 0.21568627451f, 0.38039215686f };
const static float parking_clr[] = {0.38823529411f, 0.38823529411f, 0.38823529411f};
const static float parking_nite_clr[] = { 0.13333333333f, 0.24705882352f, 0.40392156862f };
const static float regular_clr[] = {1.0f, 10.f, 10.f};
const static float heavy_clr[] = {0.99215686274f, 0.56862745098f, 0.15294117647f};
const static float button_clr_d[] = {0.22745098039f, 0.22745098039f, 0.22745098039f};
const static float button_clr_l[] = {0.32549019607f, 0.32549019607f, 0.32549019607f};
const static float button_bg[] = {0.44705882352f, 0.44705882352f, 0.44705882352f};
const static float button_text_clr[] = {0.80588235294f, 0.80588235294f, 0.80588235294f};
const static float text_error_clr[] = { 0.90588235294f, 0.0f, 0.0f };
const static float text_system_clr[] = { 0.90588235294f, 0.90588235294f, 0.0f };
const static float text_atc_clr[] = { 0.0f, 0.5f, 0.0f };
const static float callsign_clr[] = {0.83921568627f, 0.83921568627f, 0.31372549019f};
const static float conf_clr[] = {0.00392156862f, 0.72549019607f, 0.00392156862f};
const static float unknown_clr[] = {0.0f, 0.85490196078f, 0.81176470588f};
const static float collision_clr[] = { 0.00392156862f, 0.72549019607f, 0.00392156862f };
const static float col_brdr_clr[] = { 0.00392156862f, 0.92549019607f, 0.00392156862f };
const static float title_clr[] = { 0.78431372549f, 0.78431372549f, 0.78431372549f };

void InitOpenGL(GLvoid);
void ResizeMirrorGLScene(Mirror& mirror);
void ResizeGLScene();
void ResizeInterfaceGLScene();
void ResizeMirrorGLScene(Mirror*);
void SetView(double, double);
void DrawGLScene();
void DrawInterfaces();
void DrawMirrorScenes(Mirror&);
void SetPixelFormat(HDC);
int CreateSectorColours();
int DrawSceneryData(Mirror* mirror);
void RenderLegend();
void RenderButtons();
void RenderInterface(InterfaceFrame* frame);
void RenderMirrorLines(Mirror& mirror);
void RenderConf();
void RenderDate();
void RenderDepartures();
void GetOGLPos(int, int, double*);
void BuildFont(LPCWSTR, int, bool, unsigned int*, HFONT*);
void glPrint(const char*, unsigned int*, ...);
void deleteTess();
SIZE getTextExtent(std::string &s);
void DrawVarLine(float, float, float, float, float, float);
void preFileRender();

void CallInterfaces();

void CallDrawings();

void CallFocuses();

void CallLabels(InterfaceFrame* frame, Label* label);

void CallInputTexts(InterfaceFrame* frame, InputField* field);

void CallInputCursor(InterfaceFrame* frame, InputField* field);

void RenderInterfaceInputText(bool do_all);

const std::string* currentDateTime();

#endif
