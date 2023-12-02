#include <boost/date_time/posix_time/posix_time.hpp>
#include <thread>
#include <chrono>
#include <algorithm>
#include <limits>
#include <cstdarg>
#include <unordered_map>

#define NOMINMAX


#include "projection.h"
#include "renderer.h"

#include <gl/GLU.h>

#include "point2d.h"
#include "topbutton.h"
#include "tools.h"
#include "gui.h"
#include "dxfdrawing.h"
#include "aircraft.h"
#include "events.h"
#include "flightplan.h"
#include "interfaces.h"
#include "tempdata.h"

std::unordered_map<std::string, Mirror*> mirrors_storage;
std::vector<Mirror*> mirrors;

std::vector<double*> closures;

std::unordered_map<std::string, bool> rendererFlags = {
	{"sector", false},
	{"sectorColours", false},
	{"redrawClosures", false},
	{"buttons", false},
	{"legend", false},
	{"renderInterfaces", false},
	{"renderConf", false},
	{"renderDate", false},
	{"focus", false},
	{"drawings", false},
	{"queueDeleteInterface", false},
	{"renderDepartures", false},
	{"renderAllInputText", false},
	{"renderLineVis", false},
	{"renderCoordinates", false}
};

bool updateFlags[NUM_FLAGS];
bool renderFlags[NUM_FLAGS];

bool isPanning = false;
bool resize = false;
GLint sectorDl, runwaysDl, taxiwaysDl, parkingDl, apronDl, holesDl, legendDl, buttonsDl, confDl, dateDl, aircraftDl, heavyDl, unkTarDl, departuresDl, lineVisDl = 0, closureAreaList, coordinateDl;
unsigned int callSignBase, topButtonBase, confBase, legendBase, titleBase, labelBase, errorBase;
HFONT callSignFont = NULL, topBtnFont = NULL, confFont = NULL, legendFont = NULL, titleFont = NULL, labelFont = NULL,
errorFont = NULL;

std::string heavy_text = "Heavy Aircraft";
std::string regular_text = "Regular Aircraft";

void RenderChatInterface(BasicInterface&);
void RenderMirrorBorder(Mirror& mirror);
void RenderInputText(InputField&, int&, std::string&, bool);
void RenderInputCursor(InputField& input, int& inputCursorDl, std::string& text, bool centered);
void RenderLabel(BasicInterface&, int&, std::string&, int);
void RenderFocuses(InterfaceFrame* frame);
void RenderDrawings(InterfaceFrame* frame);
int RenderAircraft(bool, int&);
int RenderUnknown(bool, int&);
int RenderCallsign(Aircraft&, bool, float, float);
int RenderCollisionTag(Aircraft& aircraft, bool heavy, float latitude, float longitude);
void deleteFrame(InterfaceFrame*);
void compileClosureAreaList(const std::unordered_map<int, ClosureArea>& closureAreas);
void renderClosureArea(const ClosureArea& area, float spacing);

void set_projection(int clientWidth, int clientHeight, double c_lat, double c_lon, double zoom, double rotate, bool top_bar);

void set_projection(Mirror* mirror, int clientWidth, int clientHeight, double c_lat, double c_lon, double zoom, double rotate, bool top_bar);

int DrawCircle(Aircraft& aircraft, bool heavy, double cx, double cy, int num_segments);

int DrawCollisionLine(Mirror* mirror, Collision& collision, unsigned int& line_dl, double zoom);

int DrawVectorLines(Aircraft& aircraft, unsigned int& base, Mirror* mirror, double zoom);

void aircraft_window_data(Aircraft& aircraft, Mirror* mirror);

void aircraft_graphics(Aircraft& aircraft, Mirror* mirror);

void collision_graphics(Collision& collision, Mirror* mirror);

void updateCollisionLine(Mirror* mirror, Collision& aircraft, unsigned int& base, double zo);

void draw_point(double x, double y, ...);


std::vector<GLdouble*> tesses;

float latitude = 0, longitude = 0;
double heading = 0;
double normMapScaleX, normMapScaleY;
double minX = 0, maxX = 0, minY = 0, maxY = 0;
double circle_size = 7, tag_line_sep = 0.4, tag_line_sep2 = 0.7, config_line_sep = 0.1, dep_line_sep = 0.3;

void CALLBACK beginCallback(GLenum);
void CALLBACK endCallback(void);
void CALLBACK errorCallback(GLenum);
void CALLBACK vertexCallback(GLvoid*);
void CALLBACK combineCallback(GLdouble* const, GLdouble** const, GLfloat* const, GLdouble**);

void InitOpenGL(GLvoid) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Set the blend function
	glShadeModel(GL_SMOOTH);                            // Enable Smooth Shading
	glClearColor(day_background[0], day_background[1], day_background[2], 1.0f);    // Dark Blue Background
	//glClearDepth(1.0f);                                    // Depth Buffer Setup
	//glEnable(GL_DEPTH_TEST);                            // Enables Depth Testing
	glDisable(GL_DEPTH_TEST);                            // Disables Depth Testing
	glEnable(GL_SCISSOR_TEST);
	//glDepthFunc(GL_LEQUAL);                                // The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);    // Really Nice Perspective Calculations
	glDrawBuffer(GL_FRONT_AND_BACK);

	BuildFont(L"SansSerif", -15, true, &legendBase, &legendFont);
	BuildFont(L"Consolas", -11, false, &topButtonBase, &topBtnFont);
	BuildFont(L"Times New Roman", -15, true, &titleBase, &titleFont);
	BuildFont(L"Consolas", -13, false, &confBase, &confFont);
	BuildFont(L"Courier New", -12, false, &callSignBase, &callSignFont);
	BuildFont(L"Times New Roman", -11, true, &labelBase, &labelFont);
	BuildFont(L"Consolas", -11, false, &errorBase, &errorFont);
}

void ResizeMirrorGLScene(Mirror& mirror) {
	// Set up the viewport.
	int width = mirror.getWidth();
	int height = mirror.getHeight();
	if (width <= 0)
		width = 1;
	if (height <= 0)
		height = 1;
	//glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(mirror.getX(), mirror.getY(), width, height);
	glScissor(mirror.getX(), mirror.getY(), width, height);
	glMatrixMode(GL_PROJECTION);                        // Select The Projection Matrix
	glLoadIdentity();                                    // Reset The Projection Matrix
	// Clamp the zoom.
	double zoom = mirror.getZoom();

	set_projection(&mirror, width, height, mirror.getLat(), mirror.getLon(), zoom, mirror.getRotation(), false); // we keep the top bar so we can keep the aspect ratio

	glMatrixMode(GL_MODELVIEW);                            // Select The Modelview Matrix
	glLoadIdentity();                                    // Reset The Modelview Matrix

	SetView(mirror.getLat(), mirror.getLon());
}

void ResizeGLScene() {
	// Set up the viewport.
	int w_height = CLIENT_HEIGHT;

	//BUTTON_HEIGHT = (BUTTON_LAYOUT_HEIGHT * w_height);

	int clientWidth = CLIENT_WIDTH;
	int clientHeight = CLIENT_HEIGHT;
	if (clientWidth <= 0)
		clientWidth = 1;
	if (clientHeight <= 0)
		clientHeight = 1;
	glViewport(0, 0, clientWidth, clientHeight);
	glScissor(0, 0, clientWidth, clientHeight);

	glMatrixMode(GL_PROJECTION);                        // Select The Projection Matrix
	glLoadIdentity();                                    // Reset The Projection Matrix

	set_projection(clientWidth, clientHeight, CENTER_LAT, CENTER_LON, mZoom, rotation, true);

	glMatrixMode(GL_MODELVIEW);                            // Select The Modelview Matrix
	glLoadIdentity();                                   // Reset The Modelview Matrix

	SetView(CENTER_LAT, CENTER_LON);

	HandleMessageQueue();
}

void ResizeInterfaceGLScene() {
	int w_width = CLIENT_WIDTH, w_height = CLIENT_HEIGHT;

	glViewport(0, 0, w_width, w_height);
	glScissor(0, 0, w_width, w_height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D(0.0, CLIENT_WIDTH, 0.0, CLIENT_HEIGHT);

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	//std::cout << viewport[0] << ", " << viewport[1] << ", " << viewport[2] << ", " <<  viewport[3] << ", " << std::endl;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ResizeDataGLScene() {
	int w_width = CLIENT_WIDTH, w_height = CLIENT_HEIGHT;

	glViewport(0, 0, w_width, w_height);
	glScissor(0, 0, w_width, w_height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D(0.0, CLIENT_WIDTH, 0.0, CLIENT_HEIGHT);

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	//std::cout << viewport[0] << ", " << viewport[1] << ", " << viewport[2] << ", " <<  viewport[3] << ", " << std::endl;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ResizeMirrorDataGLScene(Mirror& mirror) {

	glViewport(mirror.getX(), mirror.getY(), mirror.getWidth(), mirror.getHeight());
	glScissor(mirror.getX(), mirror.getY(), mirror.getWidth(), mirror.getHeight());

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D(mirror.getX(), mirror.getX() + mirror.getWidth(), mirror.getY(), mirror.getY() + mirror.getHeight());

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	//std::cout << viewport[0] << ", " << viewport[1] << ", " << viewport[2] << ", " <<  viewport[3] << ", " << std::endl;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


void SetView(double latitude, double longitude) {
	gluLookAt(longitude, latitude, 0, longitude, latitude, -1, 0, 1, 0);
}

void DrawGLScene() {
	glClear(GL_COLOR_BUFFER_BIT);
	if (DAY)
		glClearColor(day_background[0], day_background[1], day_background[2], 0.0f);
	else
		glClearColor(nite_background[0], nite_background[1], nite_background[2], 0.0f);
	if (rendererFlags["sector"]) {
		glDeleteLists(taxiwaysDl, 1);
		glDeleteLists(apronDl, 1);
		glDeleteLists(runwaysDl, 1);
		glDeleteLists(parkingDl, 1);
		glPushMatrix();
		DrawSceneryData(nullptr);
		glPopMatrix();
		rendererFlags["sector"] = false;
	}

	if (rendererFlags["sectorColours"])
	{
		glDeleteLists(sectorDl, 1);
		CreateSectorColours();
		rendererFlags["sectorColours"] = false;
	}

	glCallList(sectorDl);

	if (acf_map.size() > 0) {
		for (auto iter = acf_map.begin(); iter != acf_map.end(); iter++) {
			// iterator->first = key
			Aircraft* aircraft = iter->second;
			if (aircraft != NULL) {
				//double coords[3]{ 0, 0, 0 };
				//GetWndPos(aircraft->getLatitude(), aircraft->getLongitude(), coords);
				//std::cout << aircraft->getLongitude() << std::endl;
				//std::cout << coords[0] << ", " << coords[1] << std::endl;

				aircraft_graphics(*aircraft, nullptr);
			}
		}
	}

	if (rendererFlags["redrawClosures"])
	{
		glDeleteLists(closureAreaList, 1);
		compileClosureAreaList(closureAreas);
		rendererFlags["redrawClosures"] = false;
	}

	renderAllClosureAreas();

	if (rendererFlags["renderLineVis"])
	{
		if (glIsList(lineVisDl)) {
			glDeleteLists(lineVisDl, 1);
		}

		lineVisDl = glGenLists(1);

		glNewList(lineVisDl, GL_COMPILE);

		glColor3f(1.0f, 1.0f, 1.0f);

		glBegin(GL_LINE_LOOP);
		for (const LatLon& point : debug_vis.getPoints()) {
			glVertex2f(point.lon, point.lat);
		}
		glVertex2f(DISPLAY_MOUSE_POS->x_, DISPLAY_MOUSE_POS->y_);
		glEnd();

		glEndList();

		rendererFlags["renderLineVis"] = false;
	}

	renderDebugLine();
}

void DrawData()
{

	if (acf_map.size() > 0)
	{
		//TODO Need to synch this with aircraft deletion
		for (auto iter = acf_map.begin(); iter != acf_map.end(); iter++)
		{
			// iterator->first = key
			Aircraft* acf_ptr = iter->second;
			if (acf_ptr != NULL)
			{
				Aircraft& aircraft = *acf_ptr;

				if (within_boundary(aircraft))//TODO Synch as mentioned above as this triggered breaking point
					aircraft_window_data(aircraft, nullptr);
			}
		}
	}

	if (Collision_Map.size() > 0) {
		for (auto it = Collision_Map.begin(); it != Collision_Map.end(); ++it) {
			Collision* collision = it->second;
			if (collision != NULL) {
				collision_graphics(*collision, nullptr);
			}
		}
	}
}

void DrawMirrorScenes(Mirror& mirror)
{
	glClear(GL_COLOR_BUFFER_BIT);
	if (DAY)
		glClearColor(day_background[0], day_background[1], day_background[2], 0.0f);
	else
		glClearColor(nite_background[0], nite_background[1], nite_background[2], 0.0f);


	glCallList(sectorDl);

	if (acf_map.size() > 0) {
		for (auto iter = acf_map.begin(); iter != acf_map.end(); iter++) {
			// iterator->first = key
			Aircraft* aircraft = iter->second;
			if (aircraft != NULL) {
				aircraft_graphics(*aircraft, &mirror);
			}
		}
	}

	if (mirror.renderBorder) {
		glDeleteLists(mirror.borderDl, 1);
		RenderMirrorLines(mirror);
		mirror.renderBorder = false;
	}
	glCallList(mirror.borderDl);

	renderAllClosureAreas();

	renderDebugLine();
}

void DrawMirrorData(Mirror& mirror)
{
	if (acf_map.size() > 0) {
		for (auto iter = acf_map.begin(); iter != acf_map.end(); iter++)
		{
			// iterator->first = key
			Aircraft* acf_ptr = iter->second;
			if (acf_ptr != NULL)
			{
				Aircraft& aircraft = *acf_ptr;

				//TODO Move mirrors Data to their own small projections because of text clipping
				if (mirror.within_boundary(aircraft))
				{
					aircraft_window_data(aircraft, &mirror);
				}
			}
		}
	}

	if (Collision_Map.size() > 0) {
		for (auto it = Collision_Map.begin(); it != Collision_Map.end(); ++it) {
			Collision* collision = it->second;
			if (collision != NULL) {
				collision_graphics(*collision, &mirror);
			}
		}
	}
}



void DrawInterfaces() {

	if (rendererFlags["legend"]) {
		glDeleteLists(legendDl, 1);
		RenderLegend();
		rendererFlags["legend"] = false;
	}
	glPushMatrix();
	glCallList(legendDl);
	glPopMatrix();
	if (rendererFlags["buttons"]) {
		glDeleteLists(buttonsDl, 1);
		RenderButtons();
		rendererFlags["buttons"] = false;
	}
	glPushMatrix();
	glCallList(buttonsDl);
	glPopMatrix();
	if (rendererFlags["renderInterfaces"]) {
		for (InterfaceFrame* frame : rendered_frames) {
			if (frame)
			{
				if (frame->interfaceDl != 0)
				{
					glDeleteLists(frame->interfaceDl, 1);
					frame->interfaceDl = 0;
				}
				RenderInterface(frame);
			}
		}
		rendererFlags["renderInterfaces"] = false;
	}
	CallInterfaces();
	if (rendererFlags["drawings"]) {
		//TODO add optimizations (refresh per object basis)
		for (InterfaceFrame* frame : rendered_frames)
		{
			if (frame)
			{
				if (frame->drawingDl != 0)
				{
					glDeleteLists(frame->drawingDl, 1);
					frame->drawingDl = 0;
				}
				RenderDrawings(frame);
			}
		}
		rendererFlags["drawings"] = false;
	}
	CallDrawings();
	if (rendererFlags["focus"]) {
		//TODO add optimizations (refresh per object basis)
		for (InterfaceFrame* frame : rendered_frames) {
			if (frame)
			{
				if (frame->focusDl != 0)
				{
					glDeleteLists(frame->focusDl, 1);
					frame->focusDl = 0;
				}
				RenderFocuses(frame);
			}
		}
		rendererFlags["focus"] = false;
	}
	CallFocuses();

	//all input text from all frames
	RenderInterfaceInputText();

	for (InterfaceFrame* frame : rendered_frames) {
		if (frame && frame->render) {
			if (frame->renderAllLabels) {
				for (ChildFrame* child : frame->children) {
					if (child != NULL && child->type == CHILD_TYPE::LABEL_D) {
						Label* label = (Label*)child;
						if (label->labelTextDl != 0) {
							glDeleteLists(label->labelTextDl, 1);
							label->labelTextDl = 0;
						}
						BasicInterface* border = label->border;
						RenderLabel(*border, label->labelTextDl, label->input, label->centered);
					}
				}
				frame->renderAllLabels = false;
			}
			for (ChildFrame* child : frame->children) {
				if (child) {
					if (child->type == CHILD_TYPE::LABEL_D) {
						Label* label = (Label*)child;
						CallLabels(frame, label);
					}
				}
			}
		}
	}

	if (rendererFlags["renderConf"]) {
		glDeleteLists(confDl, 1);
		RenderConf();
		RenderTerminalCommands(true);
		rendererFlags["renderConf"] = false;
	}

	glPushMatrix();
	glCallList(confDl);
	glPopMatrix();

	if (rendererFlags["renderDate"]) {
		glDeleteLists(dateDl, 1);
		RenderDate();
		rendererFlags["renderDate"] = false;
	}

	if (rendererFlags["renderCoordinates"]) {
		glDeleteLists(coordinateDl, 1);
		RenderCoordinates();
		rendererFlags["renderCoordinates"] = false;
	}

	glPushMatrix();
	glCallList(dateDl);
	glCallList(coordinateDl);
	glPopMatrix();

	if (rendererFlags["renderDepartures"]) {
		glDeleteLists(departuresDl, 1);
		RenderDepartures();
		rendererFlags["renderDepartures"] = false;
	}
	if (show_departures) {
		glPushMatrix();
		glCallList(departuresDl);
		glPopMatrix();
	}
}

void SetPixelFormat(HDC hDC) {
	static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		32,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	int index = ChoosePixelFormat(hDC, &pfd);
	SetPixelFormat(hDC, index, &pfd);
}

int CreateSectorColours() {
	sectorDl = glGenLists(1);

	glNewList(sectorDl, GL_COMPILE);

	DAY ? glColor3f(taxiway_clr[0], taxiway_clr[1], taxiway_clr[2]) : glColor3f(taxiway_nite_clr[0], taxiway_nite_clr[1], taxiway_nite_clr[2]);
	glCallList(taxiwaysDl);

	DAY ? glColor3f(apron_clr[0], apron_clr[1], apron_clr[2]) : glColor3f(apron_nite_clr[0], apron_nite_clr[1], apron_nite_clr[2]);
	glCallList(apronDl);

	glColor3f(runway_clr[0], runway_clr[1], runway_clr[2]);
	glCallList(runwaysDl);

	DAY ? glColor3f(parking_clr[0], parking_clr[1], parking_clr[2]) : glColor3f(parking_nite_clr[0], parking_nite_clr[1], parking_nite_clr[2]);
	glCallList(parkingDl);

	glEndList();
	return 1;
}

int DrawSceneryData(Mirror* mirror) {
	runwaysDl = glGenLists(1);
	taxiwaysDl = glGenLists(1);
	parkingDl = glGenLists(1);
	apronDl = glGenLists(1);

	GLUtesselator* tess = gluNewTess(); // create a tessellator
	if (tess == NULL)
		return 0;  // failed to create tessellation object, return 0

	// register callback functions
	gluTessCallback(tess, GLU_TESS_BEGIN, (void(__stdcall*)(void)) & beginCallback);
	gluTessCallback(tess, GLU_TESS_END, (void(__stdcall*)(void)) & endCallback);
	gluTessCallback(tess, GLU_TESS_ERROR, (void(__stdcall*)(void)) & errorCallback);
	gluTessCallback(tess, GLU_TESS_VERTEX, (void(__stdcall*)(void)) & vertexCallback);
	gluTessCallback(tess, GLU_TESS_COMBINE, (void(__stdcall*)(void)) & combineCallback);


	std::vector<PointTess*> runways;
	std::vector<PointTess*> parking;
	std::vector<PointTess*> taxiways;
	std::vector<PointTess*> aprons;

	for (size_t i = 0; i < ALL.size(); i++) {
		PointTess* point2d = ALL[i];
		int type = point2d->get_type();
		if (type == RUNWAY) {
			runways.push_back(point2d);
		}
		else if (type == PARKING) {
			parking.push_back(point2d);
		}
		else if (type == TAXIWAY) {
			taxiways.push_back(point2d);
		}
		else if (type == APRON) {
			aprons.push_back(point2d);
		}
	}
	//order rendered counts for overlapping
	glNewList(taxiwaysDl, GL_COMPILE);
	for (PointTess* point2d1 : taxiways) {
		std::vector<LinearSegment*> coordinates11 = point2d1->get_coordinates();
		std::vector<LinearSegment*> holes11 = point2d1->get_holes();
		//std::cout << point2d1->get_coordinates()[0][0] << std::endl;
		gluTessBeginPolygon(tess, 0);
		gluTessBeginContour(tess);
		for (LinearSegment* aCoordinates11 : coordinates11) {
			double* data = aCoordinates11->pt.as_array();
			gluTessVertex(tess, data, data);
		}
		gluTessEndContour(tess);
		gluTessEndPolygon(tess);
	}
	glEndList();

	glNewList(apronDl, GL_COMPILE);
	for (PointTess* point2d1 : aprons) {
		std::vector<LinearSegment*> coordinates11 = point2d1->get_coordinates();
		std::vector<LinearSegment*> holes11 = point2d1->get_holes();
		//std::cout << point2d1->get_coordinates()[0][0] << std::endl;
		gluTessBeginPolygon(tess, 0);
		gluTessBeginContour(tess);
		for (LinearSegment* aCoordinates11 : coordinates11) {
			double* data = aCoordinates11->pt.as_array();
			gluTessVertex(tess, data, data);
		}
		gluTessEndContour(tess);
		gluTessEndPolygon(tess);
		//delete point2d;
	}
	glEndList();

	glNewList(runwaysDl, GL_COMPILE);
	for (PointTess* point2d1 : runways) {
		std::vector<LinearSegment*> coordinates11 = point2d1->get_coordinates();
		std::vector<LinearSegment*> holes11 = point2d1->get_holes();
		gluTessBeginPolygon(tess, 0);
		gluTessBeginContour(tess);
		for (LinearSegment* aCoordinates11 : coordinates11) {
			double* data = aCoordinates11->pt.as_array();
			gluTessVertex(tess, data, data);
		}
		gluTessEndContour(tess);
		gluTessEndPolygon(tess);
	}
	glEndList();

	glNewList(parkingDl, GL_COMPILE);
	for (PointTess* point2d1 : parking) {
		std::vector<LinearSegment*> coordinates11 = point2d1->get_coordinates();
		std::vector<LinearSegment*> holes11 = point2d1->get_holes();
		gluTessBeginPolygon(tess, 0);                   // with NULL data
		gluTessBeginContour(tess);
		for (LinearSegment* aCoordinates11 : coordinates11) {
			double* data = aCoordinates11->pt.as_array();
			gluTessVertex(tess, data, data);
		}
		gluTessEndContour(tess);
		gluTessEndPolygon(tess);
		/*if (DAY) {
			glColor3f(day_background[0], day_background[1], day_background[2]);
			gluTessBeginPolygon(tess, 0);
			gluTessBeginContour(tess);
			for (LinearSegment* aHoles11 : holes11) {
				double* data = aHoles11->pt.as_array();
				gluTessVertex(tess, data, data);
			}
			gluTessEndContour(tess);
			gluTessEndPolygon(tess);
		}
		else
		{
			//handle NITE
		}*/
	}
	glEndList();

	gluDeleteTess(tess);        // delete after tessellation
	deleteTess();
	return 1;
}

void RenderLegend() {
	legendDl = glGenLists(1);
	glNewList(legendDl, GL_COMPILE);


	glColor3f(heavy_clr[0], heavy_clr[1], heavy_clr[2]);
	glRasterPos2f(18, 35);
	glPrint(heavy_text.c_str(), &legendBase);	// Print GL Text To The Screen

	glColor3f(regular_clr[0], regular_clr[1], regular_clr[2]);
	glRasterPos2f(18, 20);
	glPrint(regular_text.c_str(), &legendBase);	// Print GL Text To The Screen

	glEndList();
}

void RenderButtons() {

	int min_client_width = 1000;


	int w_width = min_client_width > CLIENT_WIDTH ? min_client_width : CLIENT_WIDTH, w_height = CLIENT_HEIGHT;

	//BUTTON_HEIGHT = (BUTTON_LAYOUT_HEIGHT * w_height);// TODO apply to native scaling (native resolution y)

	buttonsDl = glGenLists(1);

	glNewList(buttonsDl, GL_COMPILE);

	glColor3f(button_bg[0], button_bg[1], button_bg[2]);
	glBegin(GL_POLYGON);
	glVertex2d(0, CLIENT_HEIGHT);
	glVertex2d(0, (CLIENT_HEIGHT - BUTTON_HEIGHT));
	glVertex2d(w_width, (CLIENT_HEIGHT - BUTTON_HEIGHT));
	glVertex2d(w_width, CLIENT_HEIGHT);
	//std::cout << BUTTON_HEIGHT << std::endl;
	glEnd();

	int last_index = -1;
	int last_x = 0;
	int last_end_x = 0;
	int last_end_y = 0;
	for (size_t i = 0; i < BUTTONS.size(); i++) {
		TopButton& curButton = *BUTTONS[i];
		int index = curButton.getIndex();
		bool half = curButton.getIsHalf();
		bool onTop = curButton.getIsTop();
		bool isDark = curButton.getIsDark();
		bool dualOption = curButton.isDualOption();
		bool tripleOption = curButton.isTripleOption();
		int x;
		int y = 0;
		int width = BUTTON_WIDTH > (curButton.getWidth() * w_width) ? (int)BUTTON_WIDTH : (int)(curButton.getWidth() * w_width);
		int height = (int)BUTTON_HEIGHT;
		int padding = 4; // Change this to "BUTTON_PADDING" for variable padding
		if (index == last_index) {
			x = last_x;
		}
		else {
			x = last_end_x;
			if (i > 0) {
				x += (padding / 2);
			}
		}
		if (i == (BUTTONS.size() - 2) && half || i == (BUTTONS.size() - 1)) {
			width = (w_width - x);
		}
		if (half) {
			if (!onTop) {
				y = (height / 2);
			}
			else {
				y = 0;
			}
			height /= 2;
			if (!onTop) {
				y += (padding / 4);
			}
			else {
				y = 0;
			}
			height -= (padding / 4);
		}
		last_x = x;
		last_end_x = (x + width);
		last_end_y = (y + height);
		if (isDark) {
			glColor3f(button_clr_d[0], button_clr_d[1], button_clr_d[2]);
		}
		else {
			glColor3f(button_clr_l[0], button_clr_l[1], button_clr_l[2]);
		}
		glBegin(GL_POLYGON);
		glVertex2d(x, (CLIENT_HEIGHT - y));
		glVertex2d(x, (CLIENT_HEIGHT - last_end_y));
		glVertex2d(last_end_x, (CLIENT_HEIGHT - last_end_y));
		glVertex2d(last_end_x, (CLIENT_HEIGHT - y));
		glEnd();
		curButton.updateParams(x, (CLIENT_HEIGHT - y), last_end_x, (CLIENT_HEIGHT - last_end_y));
		int linePadding = 1;
		std::string first = curButton.getOption1();
		std::string second = curButton.getChoice1();
		std::string fullString;
		std::string divider = "/";
		if (second.length() > 0) {
			fullString = first + divider + second;
		}
		else {
			fullString = first;
		}
		int centerXPos = (x + (width / 2));
		if (fullString.length() > 0) {
			SelectObject(hDC, topBtnFont);
			SIZE extent = getTextExtent(fullString);
			TEXTMETRIC tm;
			GetTextMetrics(hDC, &tm);
			int tH = tm.tmAscent - tm.tmInternalLeading;
			//DeleteObject(topBtnFont);
			glColor4f(button_text_clr[0], button_text_clr[1], button_text_clr[2], 1.0f);
			int textXPos = (x + (width / 2)) - (extent.cx / 2);
			int textYPos = (CLIENT_HEIGHT - (y + (height / 2)) - (tH / 2));
			if (tripleOption) {
				int textYPos2 = (textYPos + tH) + (padding + linePadding);
				std::string text2 = curButton.getOption2();
				SIZE extent2 = getTextExtent(text2);
				int textXPos2 = (x + (width / 2)) - (extent2.cx / 2);
				glRasterPos2f((GLfloat)textXPos2, (GLfloat)textYPos2);
				glPrint(text2.c_str(), &topButtonBase);

				int textYPos3 = (textYPos - tH) - (padding + linePadding);
				std::string text3 = curButton.getOption3();
				SIZE extent3 = getTextExtent(text3);
				int textXPos3 = (x + (width / 2)) - (extent3.cx / 2);
				glRasterPos2f((GLfloat)textXPos3, (GLfloat)textYPos3);
				glPrint(text3.c_str(), &topButtonBase);
			}
			else if (dualOption) {
				int textYPos2 = textYPos - (padding + linePadding);
				std::string first2 = curButton.getOption2();
				std::string second2 = curButton.getChoice2();
				std::string fullString2;
				std::string divider = "/";
				if (second2.length() > 0) {
					fullString2 = first2 + divider + second2;
				}
				else {
					fullString2 = first2;
				}
				SIZE extent2 = getTextExtent(fullString2);
				int centerXPos = (x + (width / 2));
				int textXPos2 = centerXPos - (extent2.cx / 2);
				if (second2.length() > 0) {
					SIZE center_e = getTextExtent(divider);
					SIZE first_e = getTextExtent(first2);
					curButton.on ? glColor4f(0.87843137254f, 0.67450980392f, 0.05490196078f, 1.0f) : glColor4f(button_text_clr[0], button_text_clr[1], button_text_clr[2], 1.0f);
					glRasterPos2f((GLfloat)textXPos2, (GLfloat)textYPos2);
					glPrint(first2.c_str(), &topButtonBase);

					glColor4f((GLfloat)button_text_clr[0], (GLfloat)button_text_clr[1], (GLfloat)button_text_clr[2], (GLfloat)1.0f);
					glRasterPos2f((GLfloat)(textXPos2 + first_e.cx), (GLfloat)textYPos2);
					glPrint(divider.c_str(), &topButtonBase);

					!curButton.on ? glColor4f(0.87843137254f, 0.67450980392f, 0.05490196078f, 1.0f) : glColor4f(button_text_clr[0], button_text_clr[1], button_text_clr[2], 1.0f);
					glRasterPos2f((GLfloat)(textXPos2 + first_e.cx + center_e.cx), (GLfloat)textYPos2);
					glPrint(second2.c_str(), &topButtonBase);
				}
				else {
					glRasterPos2f((GLfloat)textXPos2, (GLfloat)textYPos2);
					glPrint(fullString2.c_str(), &topButtonBase);
				}
				textYPos += (padding + linePadding);
			}
			if (second.length() > 0) {
				SIZE center_e = getTextExtent(divider);
				SIZE first_e = getTextExtent(first);
				curButton.on ? glColor4f(0.87843137254f, 0.67450980392f, 0.05490196078f, 1.0f) : glColor4f(button_text_clr[0], button_text_clr[1], button_text_clr[2], 1.0f);
				glRasterPos2f((GLfloat)textXPos, (GLfloat)textYPos);
				glPrint(first.c_str(), &topButtonBase);

				glColor4f(button_text_clr[0], button_text_clr[1], button_text_clr[2], 1.0f);
				glRasterPos2f((GLfloat)(textXPos + first_e.cx), (GLfloat)textYPos);
				glPrint(divider.c_str(), &topButtonBase);

				!curButton.on ? glColor4f(0.87843137254f, 0.67450980392f, 0.05490196078f, 1.0f) : glColor4f(button_text_clr[0], button_text_clr[1], button_text_clr[2], 1.0f);
				glRasterPos2f((GLfloat)(textXPos + first_e.cx + center_e.cx), (GLfloat)textYPos);
				glPrint(second.c_str(), &topButtonBase);
			}
			else {
				glColor4f(button_text_clr[0], button_text_clr[1], button_text_clr[2], 1.0f);
				glRasterPos2f((GLfloat)textXPos, (GLfloat)textYPos);
				glPrint(fullString.c_str(), &topButtonBase);
			}
		}
		last_index = index;
	}
	glEndList();
}

void RenderInterface(InterfaceFrame* frame) {

	frame->interfaceDl = glGenLists(1);

	glNewList(frame->interfaceDl, GL_COMPILE);

	if (frame && frame->render) {
		if (!frame->stripped)
		{
			for (BasicInterface* inter1 : frame->interfaces) {
				if (inter1) {
					RenderChatInterface(*inter1);
					if (inter1->index == CONTENT_PANE) {
						if (frame->title.size() > 0) {
							std::string title = frame->title;
							int title_height = 25;
							double x = inter1->getStartX() + (inter1->getWidth() / 2);
							double y = (inter1->getEndY() - title_height) + (title_height / 2.0);
							SelectObject(hDC, titleFont);
							SIZE extent = getTextExtent(title);
							double textXPos = x - (extent.cx / 2.0);
							double textYPos = y - ((extent.cy / 2.0) / 2.0);
							glColor4f((GLfloat)button_text_clr[0], (GLfloat)button_text_clr[1], (GLfloat)button_text_clr[2], 1.0f);
							glRasterPos2f((GLfloat)textXPos, (GLfloat)textYPos);
							glPrint(title.c_str(), &titleBase);

							//title border line
							glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
							int b_offset_Y = 25;
							glBegin(GL_LINES);
							glVertex2f((GLfloat)(inter1->getStartX() + inter1->getWidth()), (GLfloat)(inter1->getStartY() + (inter1->getHeight() - b_offset_Y)));
							glVertex2f((GLfloat)inter1->getStartX(), (GLfloat)(inter1->getStartY() + (GLfloat)(inter1->getHeight() - b_offset_Y)));
							glEnd();
						}
					}
				}
			}
		}
		for (ChildFrame* children : frame->children) {
			if (children) {
				for (BasicInterface* inter2 : children->child_interfaces) {
					if (inter2->isRender()) {
						if (children->show_border)
							RenderChatInterface(*inter2);
					}
				}
			}
		}
	}

	glEndList();
}

void RenderMirrorLines(Mirror& mirror) {
	int width = mirror.getWidth();
	int height = mirror.getHeight();

	mirror.borderDl = glGenLists(1);

	glNewList(mirror.borderDl, GL_COMPILE);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, width, 0.0, height);

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	//std::cout << viewport[0] << ", " << viewport[1] << ", " << viewport[2] << ", " <<  viewport[3] << ", " << std::endl;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glLineWidth((GLfloat)2.0f);

	RenderMirrorBorder(mirror);

	glLineWidth((GLfloat)1.0f);

	//Restore old ortho
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glEndList();
}

void RenderDrawings(InterfaceFrame* frame) {

	frame->drawingDl = glGenLists(1);

	glNewList(frame->drawingDl, GL_COMPILE);

	if (frame && frame->render) {
		for (ChildFrame* children : frame->children) {
			if (children) {
				children->doDrawing();
			}
		}
	}

	glEndList();
}

void RenderChatInterface(BasicInterface& interface1) {
	float col[3];
	interface1.getColor(col);
	glColor4f(col[0], col[1], col[2], interface1.getTransparency());
	if (interface1.isWireMode()) {
		glBegin(GL_LINE_LOOP);
	}
	else {
		glBegin(GL_POLYGON);
	}
	glVertex2d(interface1.getStartX(), interface1.getStartY());
	glVertex2d(interface1.getStartX(), interface1.getEndY());
	glVertex2d(interface1.getEndX(), interface1.getEndY());
	glVertex2d(interface1.getEndX(), interface1.getStartY());
	glEnd();
}

void RenderMirrorBorder(Mirror& mirror) {
	glColor4f(0.5f, 0.5f, 0.5f, 1.0);
	glBegin(GL_LINE_LOOP);
	glVertex2d(1, 1);
	glVertex2d(1, mirror.getHeight() - 1);
	glVertex2d(mirror.getWidth() - 1, mirror.getHeight() - 1);
	glVertex2d(mirror.getWidth() - 1, 1);
	glEnd();
}


void CALLBACK beginCallback(GLenum which)
{
	glBegin(which);
}

void CALLBACK endCallback(void)
{
	glEnd();
}

void CALLBACK errorCallback(GLenum errorCode)
{
	const GLubyte* estring;

	estring = gluErrorString(errorCode);
	fprintf(stderr, "Tessellation Error: %s\n", estring);
	//exit (0);
}

void CALLBACK vertexCallback(GLvoid* vertex)
{
	const GLdouble* pointer;

	pointer = (GLdouble*)vertex;
	glVertex2dv(pointer);
}

void CALLBACK combineCallback(GLdouble coords[3],
	GLdouble* vertex_data[4],
	GLfloat weight[4], GLdouble** dataOut)
{
	GLdouble* vertex = new GLdouble[6];
	vertex[0] = coords[0];
	vertex[1] = coords[1];
	vertex[2] = coords[2];
	for (int i = 3; i < 6; i++) {
		vertex[i] = weight[0] * vertex_data[0][i]
			+ weight[1] * vertex_data[1][i];
		+weight[2] * vertex_data[2][i];
		+weight[3] * vertex_data[3][i];
	}
	*dataOut = vertex;
	tesses.push_back(vertex);
}

void GetOGLPos(int x, int y, double* output) {
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winX, winY, winZ = 0.0f;

	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	winX = (float)x;
	winY = (float)viewport[3] - (float)y;

	glReadPixels((int)x, (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

	gluUnProject(winX, winY, winZ, modelview, projection, viewport, &output[1], &output[0], &output[2]);
}

void GetWndPos(double latitude, double longitude, double* output) {
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];

	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	gluProject(longitude, latitude, 0.0f, modelview, projection, viewport, &output[1], &output[0], &output[2]);

	output[2] = viewport[3] - output[2];
}

void BuildFont(LPCWSTR font_name, int font_height, bool bold, unsigned int* base, HFONT* oldfont1)								// Build Our Bitmap Font
{
	HFONT	font;										// Windows Font ID
	HFONT	oldfont;

	*base = glGenLists(96);								// Storage For 96 Characters

	font = CreateFont(font_height,							// Height Of Font
		0,								// Width Of Font
		0,								// Angle Of Escapement
		0,								// Orientation Angle
		bold ? FW_BOLD : FW_REGULAR,	// Font Weight
		FALSE,							// Italic
		FALSE,							// Underline
		FALSE,							// Strikeout
		DEFAULT_CHARSET,					// Character Set Identifier
		OUT_DEFAULT_PRECIS,					// Output Precision
		CLIP_DEFAULT_PRECIS,			// Clipping Precision
		DEFAULT_QUALITY,			// Output Quality
		DEFAULT_PITCH,		// Family And Pitch
		font_name);					// Font Name

	oldfont = (HFONT)SelectObject(hDC, font);           // Selects The Font We Want
	wglUseFontBitmaps(hDC, 32, 96, *base);				// Builds 96 Characters Starting At Character 32
	*oldfont1 = (HFONT)SelectObject(hDC, oldfont);							// Selects The Font We Want
	//DeleteObject(font);									// Delete The Font
}

GLvoid KillFont(GLuint* base)									// Delete The Font List
{
	glDeleteLists(*base, 96);							// Delete All 96 Characters
}

void glPrint(const char* fmt, unsigned int* base, ...)					// Custom GL "Print" Routine
{
	char		text[256];								// Holds Our String
	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
	vsprintf_s(text, fmt, ap);						// And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text

	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	glListBase(*base - 32);								// Sets The Base Character to 32
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits
}

void deleteTess() {
	for (size_t i = 0; i < tesses.size(); i++) {
		delete[] tesses[i];
	}
	tesses.clear();
}

SIZE getTextExtent(const std::string& s) {
	std::wstring ws = s2ws(s);
	LPCWSTR wText2 = ws.c_str();
	SIZE extent;
	GetTextExtentPoint32(hDC, wText2, lstrlenW(wText2), &extent);
	return extent;
}

void RenderInputText(InputField& input, int& inputTextDl, std::string& text, bool centered) {
	BasicInterface& border = *input.border;
	int x, y = border.getStartY() + input.offset_y;
	double aW = border.getActualWidth();
	if (centered) {
		x = (border.getStartX() + (aW / 2));
	}
	else {
		x = border.getStartX() + input.offset_x;
	}

	inputTextDl = glGenLists(1);

	glNewList(inputTextDl, GL_COMPILE);

	glColor4f(input.rgb[0], input.rgb[1], input.rgb[2], 1.0f);
	SelectObject(hDC, input.font);
	SIZE size = getTextExtent(text);
	if (centered) {
		x -= (size.cx / 2);
	}
	glRasterPos2f(x, y);
	std::string finalTxt = text.c_str();
	size_t pos = finalTxt.find("%");
	while (pos != std::string::npos) {
		finalTxt.insert(pos, "%");
		pos = finalTxt.find("%", pos + 2);
	}
	glPrint(finalTxt.c_str(), input.base);

	glEndList();
}

void RenderInputCursor(InputField& input, int& inputCursorDl, std::string& text, bool centered) {
	BasicInterface& border = *input.border;
	int x, y = border.getStartY() + input.offset_y;
	double aW = border.getActualWidth();
	if (centered) {
		x = (int)(border.getStartX() + (aW / 2));
	}
	else {
		x = (int)(border.getStartX() + input.offset_x);
	}

	inputCursorDl = glGenLists(1);

	glNewList(inputCursorDl, GL_COMPILE);

	glColor4f(input.rgb[0], input.rgb[1], input.rgb[2], 1.0f);
	SelectObject(hDC, input.font);
	SIZE size = getTextExtent(text);
	if (centered) {
		x -= (size.cx / 2);
	}
	glRasterPos2f((GLfloat)x, (GLfloat)y);
	std::string finalTxt = text.c_str();
	size_t pos = finalTxt.find("%");
	while (pos != std::string::npos) {
		finalTxt.insert(pos, "%");
		pos = finalTxt.find("%", pos + 2);
	}
	glPrint(finalTxt.c_str(), input.base);

	glEndList();
}

void RenderLabel(BasicInterface& border, int& labelDl, std::string& text, int centered) {
	int x, y = border.getStartY() + 6;
	double aW = border.getActualWidth();
	if (centered == 1) {
		x = (border.getStartX() + (aW / 2));
	}
	else if (centered == 2) {
		x = border.getStartX() + aW;
	}
	else {
		x = border.getStartX() + 3;
	}

	labelDl = glGenLists(1);

	glNewList(labelDl, GL_COMPILE);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	SelectObject(hDC, labelFont);
	SIZE size = getTextExtent(text);
	if (centered == 1) {
		x -= (size.cx / 2);
	}
	else if (centered == 2) {
		x -= size.cx;
	}
	glRasterPos2f(x, y);
	std::string finalTxt = text.c_str();

	size_t pos = finalTxt.find("%");
	while (pos != std::string::npos) {
		finalTxt.insert(pos, "%");
		pos = finalTxt.find("%", pos + 2);
	}
	glPrint(finalTxt.c_str(), &labelBase);

	glEndList();
}

void RenderConf() {
	confDl = glGenLists(1);

	glNewList(confDl, GL_COMPILE);

	glColor4f(conf_clr[0], conf_clr[1], conf_clr[2], 1.0f);
	SelectObject(hDC, confFont);

	double linesX = 0, linesY = 0;

	std::string config = "RWY CONFIG: ";
	for (auto& s : logic)
	{
		config += s;
	}
	if (logic.size() > 0)
		config += "-00";

	SIZE size = getTextExtent(config);
	glRasterPos2f(30.0f, (GLfloat)(CLIENT_HEIGHT - (CLIENT_HEIGHT / 6.0)) - linesY);
	glPrint(config.c_str(), &confBase);
	linesY += (size.cy - (size.cy * config_line_sep));

	std::string config2 = "OVERLOAD END";
	size = getTextExtent(config2);
	glRasterPos2f(30.0f, (GLfloat)(CLIENT_HEIGHT - (CLIENT_HEIGHT / 6.0)) - linesY);
	glPrint(config2.c_str(), &confBase);
	linesY += (size.cy - (size.cy * config_line_sep));

	std::string config3 = "ACID" + (ASEL ? " [" + ASEL->getCallsign() + "]" : "");
	size = getTextExtent(config3);
	glRasterPos2f(30.0f, (GLfloat)(CLIENT_HEIGHT - (CLIENT_HEIGHT / 6.0)) - linesY);
	glPrint(config3.c_str(), &confBase);
	linesY += (size.cy - (size.cy * config_line_sep));

	CONF_Y = linesY;

	glEndList();
}

void RenderDate() {
	dateDl = glGenLists(1);

	glNewList(dateDl, GL_COMPILE);


	glColor4f(conf_clr[0], conf_clr[1], conf_clr[2], 1.0f);
	SelectObject(hDC, confFont);

	const std::string* date1 = currentDateTime();
	glRasterPos2f(50, (CLIENT_HEIGHT / 6.0));
	glPrint(date1[0].c_str(), &confBase);

	//std::string date2 = "1645/22";
	glRasterPos2f(53, (CLIENT_HEIGHT / 6.0) - 12);
	glPrint(date1[1].c_str(), &confBase);
	glEndList();
}

void RenderCoordinates() {
	coordinateDl = glGenLists(1);

	glNewList(coordinateDl, GL_COMPILE);


	glColor4f(conf_clr[0], conf_clr[1], conf_clr[2], 1.0f);
	SelectObject(hDC, confFont);

	std::string str = std::to_string(DISPLAY_MOUSE_POS->y_) + " : " + std::to_string(DISPLAY_MOUSE_POS->x_);
	glRasterPos2f((CLIENT_WIDTH - 160), 140);
	glPrint(str.c_str(), &confBase);

	glEndList();
}

void RenderDepartures() {
	int start_x = CLIENT_WIDTH - 30;

	departuresDl = glGenLists(1);

	glNewList(departuresDl, GL_COMPILE);

	double linesX = 0, linesY = 0;

	glColor4f(conf_clr[0], conf_clr[1], conf_clr[2], 1.0f);
	SelectObject(hDC, confFont);

	std::string config = "DEPARTURES";
	SIZE size1 = getTextExtent(config);
	std::string config2 = "---------------------";
	SIZE size2 = getTextExtent(config2);

	glRasterPos2f((start_x - (size1.cx * 0.5)) - (size2.cx * 0.5), (CLIENT_HEIGHT - (CLIENT_HEIGHT / 6.0)) - linesY);
	glPrint(config.c_str(), &confBase);
	linesY += (size1.cy - (size1.cy * dep_line_sep));


	glRasterPos2f(start_x - size2.cx, (CLIENT_HEIGHT - (CLIENT_HEIGHT / 6.0)) - linesY);
	glPrint(config2.c_str(), &confBase);
	linesY += (size2.cy - (size2.cy * dep_line_sep));

	std::string config3, config4, config5;
	SIZE size3, size4, size5;

	for (auto it = departures.begin(); it != departures.end(); ++it) {
		std::vector<std::string> dep = it->second;
		config3 = it->first;
		switch (dep.size()) {
		case 1:
			config4 = dep[0];
			config5 = "";
			break;
		case 2:
			config4 = dep[0];
			config5 = dep[1];
			break;
		}

		size3 = getTextExtent(config3), size4 = getTextExtent(config4), size5 = getTextExtent(config5);

		float callsign_x = start_x - size2.cx, fix1_x = (start_x - (size4.cx * 0.5)) - (size2.cx * 0.5), fix2_x = start_x - size5.cx;

		glRasterPos2f(callsign_x, (CLIENT_HEIGHT - (CLIENT_HEIGHT / 6.0)) - linesY);
		glPrint(config3.c_str(), &confBase);

		glRasterPos2f(fix1_x, (CLIENT_HEIGHT - (CLIENT_HEIGHT / 6.0)) - linesY);
		glPrint(config4.c_str(), &confBase);

		glRasterPos2f(fix2_x, (CLIENT_HEIGHT - (CLIENT_HEIGHT / 6.0)) - linesY);
		glPrint(config5.c_str(), &confBase);
		linesY += (size5.cy - (size5.cy * dep_line_sep));
	}

	glEndList();
}

int RenderAircraft(bool heavy, int& acf_base) {
	acf_base = glGenLists(1);
	GLUtesselator* tess = gluNewTess(); // create a tessellator
	if (tess == NULL)
		return 0;  // failed to create tessellation object, return 0

	// register callback functions
	gluTessCallback(tess, GLU_TESS_BEGIN, (void(__stdcall*)(void)) & beginCallback);
	gluTessCallback(tess, GLU_TESS_END, (void(__stdcall*)(void)) & endCallback);
	gluTessCallback(tess, GLU_TESS_ERROR, (void(__stdcall*)(void)) & errorCallback);
	gluTessCallback(tess, GLU_TESS_VERTEX, (void(__stdcall*)(void)) & vertexCallback);
	gluTessCallback(tess, GLU_TESS_COMBINE, (void(__stdcall*)(void)) & combineCallback);

	glNewList(acf_base, GL_COMPILE);
	if (heavy) {
		glColor4f(heavy_clr[0], heavy_clr[1], heavy_clr[2], 1.0f);
	}
	else {
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}

	glRotatef(((float)0.0), 0.0f, 0.0f, -1.0f);
	glScalef(1.0f, 1.0f, 1);
	gluTessBeginPolygon(tess, 0);
	gluTessBeginContour(tess);
	std::vector<double*> coords = aircraftBlip->getCoordinates();
	for (double* coord : coords) {
		gluTessVertex(tess, coord, coord);
	}
	gluTessEndContour(tess);
	gluTessEndPolygon(tess);
	glEndList();

	gluDeleteTess(tess);        // delete after tessellation
	deleteTess();
	return acf_base;
}

int RenderUnknown(bool heavy, int& acf_base) {
	acf_base = glGenLists(1);
	GLUtesselator* tess = gluNewTess(); // create a tessellator
	if (tess == NULL)
		return 0;  // failed to create tessellation object, return 0

	// register callback functions
	gluTessCallback(tess, GLU_TESS_BEGIN, (void(__stdcall*)(void)) & beginCallback);
	gluTessCallback(tess, GLU_TESS_END, (void(__stdcall*)(void)) & endCallback);
	gluTessCallback(tess, GLU_TESS_ERROR, (void(__stdcall*)(void)) & errorCallback);
	gluTessCallback(tess, GLU_TESS_VERTEX, (void(__stdcall*)(void)) & vertexCallback);
	gluTessCallback(tess, GLU_TESS_COMBINE, (void(__stdcall*)(void)) & combineCallback);

	glNewList(acf_base, GL_COMPILE);
	if (!heavy) {
		glColor4f(unknown_clr[0], unknown_clr[1], unknown_clr[2], 1.0f);
	}
	else {
		glColor4f(unknown_clr[0], unknown_clr[1], unknown_clr[2], 1.0f);
	}

	glRotatef(((float)0.0), 0.0f, 0.0f, -1.0f);
	glScalef(1.0f, 1.0f, 1);
	gluTessBeginPolygon(tess, 0);
	gluTessBeginContour(tess);
	std::vector<double*> coords = unknownBlip->getCoordinates();
	for (double* coord : coords) {
		gluTessVertex(tess, coord, coord);
	}
	gluTessEndContour(tess);
	gluTessEndPolygon(tess);
	glEndList();

	gluDeleteTess(tess);        // delete after tessellation
	deleteTess();
	return acf_base;
}

int RenderCallsign(Aircraft& aircraft, bool heavy, float latitude, float longitude) {
	aircraft.Ccallsign = glGenLists(1);
	double a_size = get_asize(heavy, false);
	double minX = aircraftBlip->getMinX();
	double minY = aircraftBlip->getMinY();
	double maxX = aircraftBlip->getMaxX();
	double maxY = aircraftBlip->getMaxY();

	double r = (maxX > maxY ? maxX : maxY);
	//if (aircraft.isCollision()) {
	r += (maxX + circle_size);
	//}
	double vMaxX = (0 + ((r * a_size) / 2.0) + (ldr_length - 1));
	double vMaxY = (0 + ((r * a_size) / 2.0) + (ldr_length - 1));

	//Draw Callsign
	glNewList(aircraft.Ccallsign, GL_COMPILE);

	glColor4f((GLfloat)callsign_clr[0], (GLfloat)callsign_clr[1], (GLfloat)callsign_clr[2], 1.0f);

	std::string textTag = aircraft.getTextTag();

	double linesX = 0, linesY = 0;
	if (show_squawks || (aircraft.getFlightPlan()->squawkCode != aircraft.getSquawkCode()))
	{
		SelectObject(hDC, callSignFont);
		SIZE size = getTextExtent(aircraft.getSquawkCode());
		glRasterPos2f((GLfloat)vMaxX, (GLfloat)vMaxY); // set position
		glPrint(aircraft.getSquawkCode().c_str(), &callSignBase);
		linesY += size.cy * tag_line_sep2;
	}

	if (!empty(textTag))
	{
		SelectObject(hDC, callSignFont);
		SIZE size = getTextExtent(textTag);
		glRasterPos2f((GLfloat)(vMaxX + linesX), (GLfloat)(vMaxY + linesY)); // set position
		glPrint(textTag.c_str(), &callSignBase);
		linesY += size.cy * tag_line_sep2;
	}

	SelectObject(hDC, callSignFont);
	SIZE size = getTextExtent(aircraft.getCallsign());
	glRasterPos2f((GLfloat)(vMaxX + linesX), (GLfloat)(vMaxY + linesY)); // set position
	glPrint(aircraft.getCallsign().c_str(), &callSignBase);

	glEndList();
	return 1;
}

int RenderCollisionTag(Aircraft& aircraft, bool heavy, float latitude, float longitude) {
	aircraft.Ccolltext = glGenLists(1);
	double a_size = get_asize(heavy, false);
	double minX = aircraftBlip->getMinX();
	double minY = aircraftBlip->getMinY();
	double maxX = aircraftBlip->getMaxX();
	double maxY = aircraftBlip->getMaxY();

	double r = (maxX > maxY ? maxX : maxY);
	//if (aircraft.isCollision()) {
	r += (maxX + circle_size);
	//}
	double vMaxX = (0 + ((r * a_size) / 2.0) + (ldr_length - 1));
	double vMaxY = -(0 + ((r * a_size) / 2.0) + (ldr_length - 1));

	//Draw Callsign
	glNewList(aircraft.Ccolltext, GL_COMPILE);

	glColor4f((GLfloat)collision_clr[0], (GLfloat)collision_clr[1], (GLfloat)collision_clr[2], 1.0f);

	std::string collTag1 = aircraft.getCollTag1();
	std::string collTag2 = aircraft.getCollTag2();

	double linesX = 0, linesY = 0;

	if (collTag1.size() >= 1) {
		SelectObject(hDC, callSignFont);
		SIZE size = getTextExtent(collTag1);
		glRasterPos2f((GLfloat)(vMaxX + linesX), (GLfloat)(vMaxY - linesY) - (size.cy / 2.0)); // set position
		glPrint(collTag1.c_str(), &callSignBase);
		linesY += size.cy * tag_line_sep2;
	}

	if (collTag2.size() >= 1) {
		SelectObject(hDC, callSignFont);
		SIZE size = getTextExtent(collTag2);
		glRasterPos2f((GLfloat)(vMaxX + linesX), (GLfloat)(vMaxY - linesY) - (size.cy / 2.0)); // set position
		glPrint(collTag2.c_str(), &callSignBase);
		linesY += size.cy * tag_line_sep2;
	}

	SelectObject(hDC, callSignFont);
	SIZE size = getTextExtent(aircraft.getCollText());
	glRasterPos2f((GLfloat)(vMaxX + linesX), (GLfloat)(vMaxY - linesY) - (size.cy / 2.0)); // set position
	glPrint(aircraft.getCollText().c_str(), &callSignBase);

	glEndList();
	return 1;
}

const std::string* currentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	char	   buf2[80];
	_gmtime64_s(&tstruct, &now);
	strftime(buf, sizeof(buf), "%m/%d/%y", &tstruct);
	strftime(buf2, sizeof(buf2), "%H%M/%S", &tstruct);
	std::string* ret = new std::string[2];
	ret[0] = buf;
	ret[1] = buf2;
	return ret;
}

void DrawVarLine(float x1, float y1, float x2, float y2, float t1, float t2)
{
	float angle = atan2(y2 - y1, x2 - x1);
	float t2sina1 = t1 / 2 * sin(angle);
	float t2cosa1 = t1 / 2 * cos(angle);
	float t2sina2 = t2 / 2 * sin(angle);
	float t2cosa2 = t2 / 2 * cos(angle);

	glBegin(GL_TRIANGLES);
	glVertex2f(x1 + t2sina1, y1 - t2cosa1);
	glVertex2f(x2 + t2sina2, y2 - t2cosa2);
	glVertex2f(x2 - t2sina2, y2 + t2cosa2);
	glVertex2f(x2 - t2sina2, y2 + t2cosa2);
	glVertex2f(x1 - t2sina1, y1 + t2cosa1);
	glVertex2f(x1 + t2sina1, y1 - t2cosa1);
	glEnd();
}

void RenderFocuses(InterfaceFrame* frame) {
	frame->focusDl = glGenLists(1);

	glNewList(frame->focusDl, GL_COMPILE);
	if (frame && frame->render) {
		for (ChildFrame* children : frame->children) {
			if (children) {
				children->focusDrawing();
			}
		}
	}
	glEndList();
}

void set_projection(int clientWidth, int clientHeight, double c_lat, double c_lon, double zoom, double rotate, bool top_bar)
{
	set_projection(nullptr, clientWidth, clientHeight, c_lat, c_lon, zoom, rotate, top_bar);
}

void set_projection(Mirror* mirror, int clientWidth, int clientHeight, double c_lat, double c_lon, double zoom, double rotate, bool top_bar) {

	// Clamp the zoom.
	if (zoom > MAX_ZOOM) {
		zoom = MAX_ZOOM;
	}
	else if (zoom < MIN_ZOOM) {
		zoom = MIN_ZOOM;
	}

	double button_height = BUTTON_HEIGHT;

	if (!top_bar) {
		double m = (double)clientHeight / (double)CLIENT_HEIGHT;
		button_height = BUTTON_HEIGHT * m;
	}

	double comp = angularCompensation(rotation);

	// Set the min/max lat/lon based on center point, zoom, and client aspect.
	double coordSysMinLon = c_lon - ((clientWidth / 2.0) * (zoom / NM_PER_DEG));
	double coordSysMaxLon = c_lon + ((clientWidth / 2.0) * (zoom / NM_PER_DEG));
	double coordSysMinLat = c_lat - (((clientHeight + button_height) / 2.0) * (zoom / NM_PER_DEG));
	double coordSysMaxLat = c_lat + (((clientHeight + button_height) / 2.0) * (zoom / NM_PER_DEG));

	if (mirror)
	{
		mirror->setBoundaries(coordSysMinLon, coordSysMaxLon, coordSysMinLat, coordSysMaxLat);
	}
	else
	{
		minX = coordSysMinLon, maxX = coordSysMaxLon, minY = coordSysMinLat, maxY = coordSysMaxLat;
	}

	// Calculate coordinate system boundaries.
	double coordSysWidth = coordSysMaxLon - coordSysMinLon;
	double coordSysHeight = coordSysMaxLat - coordSysMinLat;
	double minX = 0.0 - (coordSysWidth / 2.0);
	double maxX = coordSysWidth / 2.0;
	double minY = 0.0 - (coordSysHeight / 2.0);
	double maxY = coordSysHeight / 2.0;

	// Set up the projection matrix based on these boundaries.
	gluOrtho2D(minX, maxX, minY, maxY);

	double nmPerLon = NauticalMilesPerDegreeLon(c_lat);
	double mapScaleX = (nmPerLon / 60.0), mapScaleY = 1.0;
	normMapScaleX = 1.0 / mapScaleX, normMapScaleY = 1.0 / mapScaleY;
	double diff = (60.0 - nmPerLon) / 60.0;

	glRotatef(rotate, 0.0f, 0.0f, -1.0f);

	glScaled(mapScaleX, mapScaleY, 1.0);
}

int DrawCircle(Aircraft& aircraft, bool heavy, double cx, double cy, int num_segments) {
	double aircraft_size = get_asize(heavy, false);
	double maxX = aircraftBlip->getMaxX();
	double maxY = aircraftBlip->getMaxY();

	double r = (maxX > maxY ? maxX : maxY);
	r += circle_size;

	aircraft.collisionDl = glGenLists(1);

	glNewList(aircraft.collisionDl, GL_COMPILE);

	glColor4f(collision_clr[0], collision_clr[1], collision_clr[2], 0.6f);

	//glTranslatef(+longitude, +latitude, 0.0f);
	//glRotatef(((float)heading), 0.0f, 0.0f, -1.0f);
	//glTranslatef(-longitude, -latitude, 0.0f);

	glBegin(GL_POLYGON);
	for (int ii = 0; ii < num_segments; ii++) {
		float theta = 2.0f * 3.1415926f * float(ii) / float(num_segments);//get the current angle 
		double x = r * cosf(theta);//calculate the x component 
		double y = r * sinf(theta);//calculate the y component 
		glVertex2f(x + cx, y + cy);//output vertex 
	}
	glEnd();

	glColor4f(col_brdr_clr[0], col_brdr_clr[1], col_brdr_clr[2], 1.0f);

	//glTranslatef(+longitude, +latitude, 0.0f);
	//glRotatef(((float)heading), 0.0f, 0.0f, -1.0f);
	//glTranslatef(-longitude, -latitude, 0.0f);
	glBegin(GL_LINE_LOOP);
	for (int ii = 0; ii < num_segments; ii++) {
		float theta = 2.0f * 3.1415926f * float(ii) / float(num_segments);//get the current angle 
		double x = r * cosf(theta);//calculate the x component 
		double y = r * sinf(theta);//calculate the y component 
		glVertex2f(x + cx, y + cy);//output vertex 
	}
	glEnd();
	glEndList();
	return 1;
}

int DrawCollisionLine(Mirror* mirror, Collision& collision, unsigned int& line_dl, double zoom) {

	Aircraft& from = *collision.getAircraft1();
	Aircraft& to = *collision.getAircraft2();

	bool is_mirror = mirror ? true : false;

	bool heavy_from = from.isHeavy(), heavy_to = to.isHeavy();

	double _aircraft_size = get_asize(heavy_from, false);
	double _aircraft_size2 = get_asize(heavy_to, false);
	double maxX = aircraftBlip->getMaxX();
	double maxY = aircraftBlip->getMaxY();

	double lon_f = is_mirror ? mirror->wndc[&from][1] : from.wnd_loc[1], lat_f = is_mirror ? mirror->wndc[&from][0] : from.wnd_loc[0];
	double lon_t = is_mirror ? mirror->wndc[&to][1] : to.wnd_loc[1], lat_t = is_mirror ? mirror->wndc[&to][0] : to.wnd_loc[0];

	double bearing_from = getBearing(from.getLatitude(), from.getLongitude(), to.getLatitude(), to.getLongitude());
	double bearing_to = getBearing(to.getLatitude(), to.getLongitude(), from.getLatitude(), from.getLongitude());

	double offset = (maxX > maxY ? maxX : maxY) + circle_size;

	Point2 p = GetVector(lat_f, lon_f, (offset * _aircraft_size), bearing_from + (float)hdg(is_mirror ? mirror->getRotation() : rotation));
	Point2 p2 = GetVector(lat_t, lon_t, offset * _aircraft_size2, bearing_to + (float)hdg(is_mirror ? mirror->getRotation() : rotation));

	//printf("coords [%f, %f] [%f %f]\n", lon_f, lon_t, bearing_from, bearing_to);

	line_dl = glGenLists(1);

	glNewList(line_dl, GL_COMPILE);

	glColor4f(col_brdr_clr[0], col_brdr_clr[1], col_brdr_clr[2], 1.0f);

	glBegin(GL_LINES);

	glVertex2f((GLfloat)p.x_, (GLfloat)p.y_);//output vertex 
	glVertex2f((GLfloat)p2.x_, (GLfloat)p2.y_);//output vertex 

	glEnd();

	glEndList();
	return 1;
}

int DrawVectorLines(Aircraft& aircraft, unsigned int& base, Mirror* mirror, double zoom)
{

	bool is_mirror = mirror ? true : false;
	double heading = aircraft.getHeading();
	bool heavy = aircraft.isHeavy();
	bool standby = aircraft.getMode() == 0 ? true : false;

	double _aircraft_size = get_asize(heavy, false);

	double maxX = aircraftBlip->getMaxX();
	double maxY = aircraftBlip->getMaxY();

	double offset = (maxX > maxY ? maxX : maxY);

	double x = is_mirror ? mirror->wndc[&aircraft][1] : aircraft.wnd_loc[1],
		y = is_mirror ? mirror->wndc[&aircraft][0] : aircraft.wnd_loc[0];

	Point2 p = GetVector(y, x, (vector_length * 3) + (offset * _aircraft_size), heading + (float)hdg(is_mirror ? mirror->getRotation() : rotation));

	base = glGenLists(1);

	glNewList(base, GL_COMPILE);

	heavy ? glColor4f(heavy_clr[0], heavy_clr[1], heavy_clr[2], 1.0f) :
		standby ? glColor4f(unknown_clr[0], unknown_clr[1], unknown_clr[2], 1.0f)
		: glColor4f(regular_clr[0], regular_clr[1], regular_clr[2], 1.0f);

	glBegin(GL_LINES);

	glVertex2f((GLfloat)x, (GLfloat)y);//output vertex 
	glVertex2f((GLfloat)p.x_, (GLfloat)p.y_);//output vertex 

	glEnd();

	glEndList();
	return 1;
}

void aircraft_window_data(Aircraft& aircraft, Mirror* mirror)
{
	bool is_mirror = mirror ? true : false;
	aircraft.lock();
	double acf_lat = aircraft.getLatitude();
	double acf_lon = aircraft.getLongitude();
	double acf_heading = aircraft.getHeading();
	bool renderCallsign = aircraft.getRenderFlag(ACF_CALLSIGN);
	bool renderCollTag = aircraft.getRenderFlag(ACF_COLLISION_TAG);
	bool renderCollision = aircraft.getRenderFlag(ACF_COLLISION);
	bool renderVector = aircraft.getRenderFlag(ACF_VECTOR);
	bool heavy = aircraft.isHeavy();
	bool standby = aircraft.getMode() == 0 ? true : false;
	aircraft.unlock();

	double zo = is_mirror ? mirror->getZoom() : mZoom;
	double a_size = get_asize(heavy, standby);

	if (!is_mirror)
	{
		if (renderCallsign || renderFlags[GBL_CALLSIGN]) {
			if (aircraft.Ccallsign != 0) {
				glDeleteLists(aircraft.Ccallsign, 1);
				aircraft.Ccallsign = 0;
			}
			RenderCallsign(aircraft, heavy, 0, 0);
		}
	}

	//Begin Callsign Drawing
	glPushMatrix();

	glTranslated(is_mirror ? mirror->wndc[&aircraft][1] : aircraft.wnd_loc[1],
		is_mirror ? mirror->wndc[&aircraft][0] : aircraft.wnd_loc[0], 0.0f);

	//keep the callsign drawing in correct aspect ratio
	//glTranslated(+longitude, +latitude, 0.0f);
	//glScaled(normMapScaleX, normMapScaleY, 1.0);
	//glTranslated(-longitude, -latitude, 0.0f);

	//scale to what ever zoom we are using (DONT UPDATE GL COMPILE LIST WHILE DOING THIS, otherwise, it will go smaller)
	//glTranslated(+longitude, +latitude, 0.0f);
	//glScaled(a_size, a_size, 1.0);
	//glTranslated(-longitude, -latitude, 0.0f);

	if (!standby && DB_BLOCK) {
		glCallList(aircraft.Ccallsign);
	}
	glPopMatrix();

	if (!is_mirror)
	{
		if (renderCollTag || renderFlags[GBL_COLLISION_TAG]) {
			if (aircraft.Ccolltext != 0)
			{
				glDeleteLists(aircraft.Ccolltext, 1);
				aircraft.Ccolltext = 0;
			}
			RenderCollisionTag(aircraft, heavy, 0, 0);
		}
	}

	if (aircraft.isCollision()) {
		glPushMatrix();

		glTranslated(is_mirror ? mirror->wndc[&aircraft][1] : aircraft.wnd_loc[1],
			is_mirror ? mirror->wndc[&aircraft][0] : aircraft.wnd_loc[0], 0.0f);

		//keep the callsign drawing in correct aspect ratio
		//glTranslated(+longitude, +latitude, 0.0f);
		//glScaled(normMapScaleX, normMapScaleY, 1.0);
		//glTranslated(-longitude, -latitude, 0.0f);

		//scale to what ever zoom we are using (DONT UPDATE GL COMPILE LIST WHILE DOING THIS, otherwise, it will go smaller)
		//glTranslated(+longitude, +latitude, 0.0f);
		//glScaled(a_size, a_size, 1.0);
		//glTranslated(-longitude, -latitude, 0.0f);

		glCallList(aircraft.Ccolltext);

		glPopMatrix();
	}

	if (!is_mirror) {
		if (renderCollision || renderFlags[GBL_COLLISION]) {
			if (aircraft.collisionDl != 0) {
				glDeleteLists(aircraft.collisionDl, 1);
				aircraft.collisionDl = 0;
			}
			DrawCircle(aircraft, heavy, longitude, latitude, 100);
		}
	}

	//begin Collision drawing
	glPushMatrix();

	glTranslated(is_mirror ? mirror->wndc[&aircraft][1] : aircraft.wnd_loc[1],
		is_mirror ? mirror->wndc[&aircraft][0] : aircraft.wnd_loc[0], 0.0f);

	//scale to what ever zoom we are using (DONT UPDATE GL COMPILE LIST WHILE DOING THIS, otherwise, it will go smaller)
	glTranslated(+longitude, +latitude, 0.0f);
	glScaled(a_size, a_size, 1.0);
	glTranslated(-longitude, -latitude, 0.0f);

	if (aircraft.isCollision())
	{
		glCallList(aircraft.collisionDl);
	}

	glPopMatrix();

	if (renderVector || renderFlags[GBL_VECTOR]
		|| (is_mirror && mirror->render_flags[MIR_VECTOR])) {
		if (is_mirror && mirror->g_flags[&aircraft][ACF_VECTOR] != 0) {
			glDeleteLists(mirror->g_flags[&aircraft][ACF_VECTOR], 1);
			mirror->g_flags[&aircraft][ACF_VECTOR] = 0;
		}
		else if (aircraft.vectorDl != 0) {
			glDeleteLists(aircraft.vectorDl, 1);
			aircraft.vectorDl = 0;
		}
		DrawVectorLines(aircraft, is_mirror ? mirror->g_flags[&aircraft][ACF_VECTOR] : aircraft.vectorDl, mirror, zo);
	}

	glPushMatrix();

	if (!standby && SHOW_VECTORS) {
		glCallList(is_mirror ? mirror->g_flags[&aircraft][ACF_VECTOR] : aircraft.vectorDl);
	}

	glPopMatrix();

	if (!is_mirror)
	{
		if (renderFlags[GBL_AIRCRAFT]) {
			//We use default zoom because they all get downsized by glScale anyway
			glDeleteLists(aircraftDl, 1);
			RenderAircraft(false, aircraftDl);
			glDeleteLists(heavyDl, 1);
			RenderAircraft(true, heavyDl);
			glDeleteLists(unkTarDl, 1);
			RenderUnknown(false, unkTarDl);
		}
	}

	//move the aircraft first so we have proper movement along the map scale
	glPushMatrix();
	glTranslated(is_mirror ? mirror->wndc[&aircraft][1] : aircraft.wnd_loc[1],
		is_mirror ? mirror->wndc[&aircraft][0] : aircraft.wnd_loc[0], 0.0f);

	//keep the aircraft drawing in correct aspect ratio
	//std::cout << normMapScaleX << std::endl;
	//glTranslated(+longitude, +latitude, 0.0f);
	//glScaled(normMapScaleX, normMapScaleY, 1.0f);
	//glTranslated(-longitude, -latitude, 0.0f);

	//scale to what ever zoom we are using (DONT UPDATE GL COMPILE LIST WHILE DOING THIS, otherwise, it will go smaller)
	glScaled(a_size, a_size, 1.0);

	//set the aircraft heading - this needs to come after scaling aspect for proper rotation
	glRotatef(((float)hdg(acf_heading + (is_mirror ? mirror->getRotation() : rotation))), 0.0f, 0.0f, -1.0f);
	if (standby) {
		glCallList(unkTarDl);
	}
	else
	{
		if (!heavy)
		{
			glCallList(aircraftDl);
		}
		else
		{
			glCallList(heavyDl);
		}
	}
	glPopMatrix();
}

void aircraft_graphics(Aircraft& aircraft, Mirror* mirror) {
	bool is_mirror = mirror ? true : false;
	aircraft.lock();
	double acf_lat = aircraft.getLatitude();
	double acf_lon = aircraft.getLongitude();
	double acf_heading = aircraft.getHeading();
	bool renderCallsign = aircraft.getRenderFlag(ACF_CALLSIGN);
	bool renderCollTag = aircraft.getRenderFlag(ACF_COLLISION_TAG);
	bool renderCollision = aircraft.getRenderFlag(ACF_COLLISION);
	bool renderVector = aircraft.getRenderFlag(ACF_VECTOR);
	bool heavy = aircraft.isHeavy();
	bool standby = aircraft.getMode() == 0 ? true : false;
	aircraft.unlock();

	double zo = is_mirror ? mirror->getZoom() : mZoom;
	double a_size = get_asize(heavy, standby);

	//convert latitude longitude to window coords
	if (is_mirror)
	{
		GetWndPos(acf_lat, acf_lon, mirror->wndc[&aircraft]);
	}
	else
	{
		GetWndPos(acf_lat, acf_lon, aircraft.wnd_loc);
	}

	//if (renderCallsign || renderFlags[GBL_CALLSIGN]) {
	//	GetCallsignCoords(aircraft, mirror, heavy, latitude, longitude);
	//}


	/*if (!is_mirror) {
		if (renderCollTag || renderFlags[GBL_COLLISION_TAG]) {
			if (aircraft.Ccolltext != 0)
			{
				glDeleteLists(aircraft.Ccolltext, 1);
				aircraft.Ccolltext = 0;
			}
			RenderCollisionTag(aircraft, heavy, latitude, longitude);
		}
	}*/

	//Begin Callsign Drawing

	/*if (aircraft.isCollision()) {
		glPushMatrix();

		glTranslated((acf_lon - longitude), (acf_lat - latitude), 0.0f);

		//keep the callsign drawing in correct aspect ratio
		glTranslated(+longitude, +latitude, 0.0f);
		glScaled(normMapScaleX, normMapScaleY, 1.0);
		glTranslated(-longitude, -latitude, 0.0f);

		//scale to what ever zoom we are using (DONT UPDATE GL COMPILE LIST WHILE DOING THIS, otherwise, it will go smaller)
		glTranslated(+longitude, +latitude, 0.0f);
		glScaled(a_size, a_size, 1.0);
		glTranslated(-longitude, -latitude, 0.0f);

		glCallList(aircraft.Ccolltext);

		glPopMatrix();
	}*/
}

void collision_graphics(Collision& collision, Mirror* mirror) {
	bool is_mirror = mirror ? true : false;
	double zo = is_mirror ? mirror->getZoom() : mZoom;

	if (is_mirror) {
		//TODO make this more efficient, we shouldn't be searching an ordermap every single frame
		if (collision.getRenderFlag(COL_COLLISION_LINE) || mirror->render_flags[MIR_COLLISION_LINE]) {
			updateCollisionLine(mirror, collision, mirror->c_flags[&collision][0], zo);
		}

		glPushMatrix();
		glCallList(mirror->c_flags[&collision][0]);
		glPopMatrix();
	}
	else {
		if (collision.getRenderFlag(COL_COLLISION_LINE) || renderFlags[GBL_COLLISION_LINE]) {
			updateCollisionLine(mirror, collision, collision.collLineDL, zo);
		}

		glPushMatrix();
		glCallList(collision.collLineDL);
		glPopMatrix();
	}
}

void updateCollisionLine(Mirror* mirror, Collision& collision, unsigned int& base, double zo) {
	if (base != 0) {
		glDeleteLists(base, 1);
		base = 0;
	}
	if (collision.getAircraft1() && collision.getAircraft2()) {
		DrawCollisionLine(mirror, collision, base, zo);
	}
}

void preFileRender() {
	aircrafts_to_mirrors();

	collisions_to_mirrors();
}

void aircrafts_to_mirrors()
{
	if (acf_map.size() > 0) {
		for (auto iter = acf_map.begin(); iter != acf_map.end(); iter++) {
			// iterator->first = key
			Aircraft* acf = iter->second;
			if (acf != NULL) {
				Aircraft& aircraft = *acf;
				double acf_lat = aircraft.getLatitude();
				double acf_lon = aircraft.getLongitude();
				double acf_heading = aircraft.getHeading();
				bool renderCallsign = aircraft.getRenderFlag(ACF_CALLSIGN);
				bool renderCollision = aircraft.getRenderFlag(ACF_COLLISION);
				bool heavy = aircraft.isHeavy();
				bool standby = aircraft.getMode() == 0 ? true : false;


				double zo = mZoom;
				double a_size = get_asize(heavy, standby);

				addAircraftToMirrors(acf);
			}
		}
	}
}

void collisions_to_mirrors()
{
	if (Collision_Map.size() > 0) {
		for (auto it = Collision_Map.begin(); it != Collision_Map.end(); ++it)
		{
			Collision* collision = it->second;
			if (collision != NULL)
			{
				glDeleteLists(collision->collLineDL, 1);

				addCollisionToMirrors(collision);
			}
		}
	}
}

void CallInterfaces() {
	for (InterfaceFrame* frame : rendered_frames) {
		if (frame && frame->render)
		{
			glPushMatrix();

			if (frame->cur_pt && frame->s_pt)
			{

				glTranslated((frame->cur_pt->x - frame->s_pt->x), -(frame->cur_pt->y - frame->s_pt->y), 0.0f);

				glCallList(frame->interfaceDl);
			}
			else
			{
				glCallList(frame->interfaceDl);
			}

			glPopMatrix();
		}
	}
}

void CallDrawings() {

	for (InterfaceFrame* frame : rendered_frames) {
		if (frame && frame->render)
		{
			glPushMatrix();

			if (frame->cur_pt && frame->s_pt)
			{

				glTranslated((frame->cur_pt->x - frame->s_pt->x), -(frame->cur_pt->y - frame->s_pt->y), 0.0f);

				glCallList(frame->drawingDl);
			}
			else
			{
				glCallList(frame->drawingDl);
			}

			glPopMatrix();
		}
	}
}

void CallFocuses() {
	for (InterfaceFrame* frame : rendered_frames) {
		if (frame && frame->render)
		{
			glPushMatrix();

			if (frame->cur_pt && frame->s_pt)
			{

				glTranslated((frame->cur_pt->x - frame->s_pt->x), -(frame->cur_pt->y - frame->s_pt->y), 0.0f);

				glCallList(frame->focusDl);
			}
			else
			{
				glCallList(frame->focusDl);
			}

			glPopMatrix();
		}
	}
}

void CallLabels(InterfaceFrame* frame, Label* label) {
	if (frame->render)
	{
		glPushMatrix();

		if (frame->cur_pt && frame->s_pt)
		{

			glTranslated((frame->cur_pt->x - frame->s_pt->x), -(frame->cur_pt->y - frame->s_pt->y), 0.0f);

			glCallList(label->labelTextDl);
		}
		else
		{
			glCallList(label->labelTextDl);
		}

		glPopMatrix();
	}
}

void CallInputTexts(InterfaceFrame* frame, InputField* field) {
	if (frame->render)
	{
		glPushMatrix();

		if (frame->cur_pt && frame->s_pt)
		{

			glTranslated((frame->cur_pt->x - frame->s_pt->x), -(frame->cur_pt->y - frame->s_pt->y), 0.0f);

			glCallList(field->inputTextDl);
		}
		else
		{
			glCallList(field->inputTextDl);
		}

		glPopMatrix();
	}
}

bool RenderChild(ChildFrame* child)
{
	if (child)
	{
		child->render = true;
		return true;
	}
	return false;
}

bool RenderChild()
{
	if (focusChild)
	{
		focusChild->render = true;
		return true;
	}
	return false;
}

bool RenderChild(ChildFrame* child, CHILD_TYPE type)
{
	if (child && child->type == type)
	{
		child->render = true;
		return true;
	}
	return false;
}

bool RenderFocusChild(CHILD_TYPE type)
{
	if (focusChild && focusChild->type == type)
	{
		focusChild->render = true;
		return true;
	}
	return false;
}

void CallInputCursor(InterfaceFrame* frame, InputField* field) {
	if (frame->render)
	{
		glPushMatrix();

		if (frame->cur_pt && frame->s_pt)
		{

			glTranslated((frame->cur_pt->x - frame->s_pt->x), -(frame->cur_pt->y - frame->s_pt->y), 0.0f);

			glCallList(field->inputCursorDl);
		}
		else
		{
			glCallList(field->inputCursorDl);
		}

		glPopMatrix();
	}
}

void RenderInterfaceInputText() {
	//frame specific input text
	for (InterfaceFrame* frame : rendered_frames) {
		if (frame && frame->render) {
			for (ChildFrame* child : frame->children)
			{
				if (child && (child->render || frame->renderAllInputText || rendererFlags["renderAllInputText"]))
				{
					if (child->type == CHILD_TYPE::INPUT_FIELD)
					{
						InputField* field = (InputField*)child;
						if (field->inputTextDl != 0) {
							glDeleteLists(field->inputTextDl, 1);
							field->inputTextDl = 0;
						}
						if (field->inputCursorDl != 0) {
							glDeleteLists(field->inputCursorDl, 1);
							field->inputCursorDl = 0;
						}
						std::string f_input;
						if (field->p_protected) {
							f_input = field->pp_input;
						}
						else {
							f_input = field->input;
						}
						RenderInputText(*field, field->inputTextDl, f_input, field->centered);
						RenderInputCursor(*field, field->inputCursorDl, field->cursor_input, field->centered);
					}
					child->render = false;
				}
			}
			for (ChildFrame* child : frame->children) {
				if (child) {
					if (child->type == CHILD_TYPE::INPUT_FIELD) {
						InputField* field = (InputField*)child;
						CallInputTexts(frame, field);
						CallInputCursor(frame, field);
					}
				}
			}
			frame->renderAllInputText = false;
		}
	}

	rendererFlags["renderAllInputText"] = false;
}

void draw_point(double x, double y, ...)
{
	double size = 10.0;
	double startX = x - size, startY = y - size;
	double endX = x + size, endY = y + size;

	va_list args;
	va_start(args, y);

	GLfloat color[4];
	for (int i = 0; i < 4; ++i) {
		color[i] = static_cast<GLfloat>(va_arg(args, double));
	}

	va_end(args);

	int line_width = 2;
	startX += line_width, startY += line_width;
	endX -= line_width, endY -= line_width;

	glColor4fv(color);

	DrawVarLine((float)startX, (float)endY, (float)endX, (float)startY, (float)line_width, (float)line_width);
	DrawVarLine((float)startX, (float)startY, (float)endX, (float)endY, (float)line_width, (float)line_width);
}

void HandleMessageQueue()
{
	for (auto it = message_queue.begin(); it != message_queue.end();)
	{
		if ((*it) != nullptr)
		{
			MSG& msg = *(*it);
			switch (msg.message)
			{
			case WM_RBUTTONDOWN:
			{
				double coords[3];
				GetOGLPos(LOWORD(msg.lParam), HIWORD(msg.lParam), coords);
				printf("\nBoundaries For Closure: %f %f\n", coords[0], coords[1]);
				if (DUMP_COLLISION)
				{
					filerdr.clickPoints.push_back(Point2(coords[1], coords[0]));
				}
				if (DUMP_CLOSURE) {
					debug_vis.addPoint(coords[0], coords[1]);
					rendererFlags["renderLineVis"] = true;
				}
			}
			break;
			case WM_LBUTTONDOWN:
			{
				isPanning = true;
				double coords[3];
				GetOGLPos(LOWORD(msg.lParam), HIWORD(msg.lParam), coords);

				MOUSE_POS->x_ = LOWORD(msg.lParam);// coords[1];
				MOUSE_POS->y_ = HIWORD(msg.lParam);//coords[0];

			}
			break;
			case WM_MOUSEMOVE:
			{
				double coords[3];
				GetOGLPos(LOWORD(msg.lParam), HIWORD(msg.lParam), coords);
				if (isPanning)
				{
					int xDiff = MOUSE_POS->x_ - LOWORD(msg.lParam);
					int yDiff = HIWORD(msg.lParam) - MOUSE_POS->y_;

					if (dragged_pos)
					{
						//double panningFactor = getPanningFactor(dragged_pos->getWidth(), dragged_pos->getHeight());
						dragged_pos->addToCoords(yDiff / PANNING_SENSITIVITY, xDiff / PANNING_SENSITIVITY);
					}

					MOUSE_POS->x_ = LOWORD(msg.lParam);
					MOUSE_POS->y_ = HIWORD(msg.lParam);
				}
				DISPLAY_MOUSE_POS->x_ = coords[1];
				DISPLAY_MOUSE_POS->y_ = coords[0];
				if (DUMP_CLOSURE)
					rendererFlags["renderLineVis"] = true;
				rendererFlags["renderCoordinates"] = true;
			}
			break;
			case WM_LBUTTONUP:
			{
				isPanning = false;
			}
			break;
			}
			*it = nullptr;
		}
		++it;
	}
}

void renderAllClosureAreas() {
	glCallList(closureAreaList);
}

void renderDebugLine() {
	glCallList(lineVisDl);
}

void compileClosureAreaList(const std::unordered_map<int, ClosureArea>& closureAreas) {
	// If the list already exists, delete it
	if (glIsList(closureAreaList)) {
		glDeleteLists(closureAreaList, 1);
	}

	// Generate a new list
	closureAreaList = glGenLists(1);
	glNewList(closureAreaList, GL_COMPILE);

	// Render each closure area
	for (const auto& area : closureAreas) {
		renderClosureArea(area.second, 0.001f);
	}

	glEndList();
}

#define MAX_NODES 1000
#define FAR_FAR_AWAY 999999999.

void drawLineSegmentABtoXY(double a, double b, double x, double y) {
	// This function will draw a line segment from (a, b) to (x, y)
	// You can use OpenGL commands here to draw the line segment.
	glBegin(GL_LINES);
	glVertex2f(a, b);
	glVertex2f(x, y);
	glEnd();
}

bool drawDiagonalStripes(long polygonCount, long* polygonCorners,
	double** polygons, double spacing) {

#define  MAX_NODES          1000
#define  FAR_FAR_AWAY  999999999.

	double  spanMin = FAR_FAR_AWAY, theCos, theSin, nodeX[MAX_NODES];
	double  spanMax = -FAR_FAR_AWAY, x, y, a, b, newX, stripeY, swap;
	long    i, j, k, spanStart, spanEnd, nodeCount, step;

	// Create a 45-degree angle for stripes.
	theCos = sqrt(.5);
	theSin = sqrt(.5);

	// Loop to determine the span over which diagonal lines must be drawn.
	for (i = 0; i < polygonCount; i++) {
		for (j = 0; j < polygonCorners[i]; j++) {
			x = polygons[i][j * 2];
			y = polygons[i][j * 2 + 1];

			// Rotate the point, since the stripes may be at an angle.
			y = y * theCos - x * theSin;

			// Adjust the span.
			if (spanMin > y) spanMin = y;
			if (spanMax < y) spanMax = y;
		}
	}

	// Turn the span into a discrete step range.
	spanStart = (long)floor(spanMin / spacing) - 1;
	spanEnd = (long)floor(spanMax / spacing) + 1;

	// Loop to create all stripes.
	for (step = spanStart; step <= spanEnd; step++) {
		nodeCount = 0;  // Initialize nodeCount here
		stripeY = spacing * (double)step;

		// Loop to build a node list for one row of stripes.
		for (i = 0; i < polygonCount; i++) {
			k = polygonCorners[i] - 1;
			for (j = 0; j < polygonCorners[i]; j++) {
				a = polygons[i][k * 2];
				b = polygons[i][k * 2 + 1];
				x = polygons[i][j * 2];
				y = polygons[i][j * 2 + 1];

				// Rotate the points, since the stripes may be at an angle.
				newX = a * theCos + b * theSin;
				b = b * theCos - a * theSin; a = newX;
				newX = x * theCos + y * theSin;
				y = y * theCos - x * theSin; x = newX;

				// Find the node, if any.
				if (b < stripeY && y >= stripeY
					|| y < stripeY && b >= stripeY) {
					if (nodeCount >= MAX_NODES) return false;
					nodeX[nodeCount++] = a + (x - a) * (stripeY - b) / (y - b);
				}

				k = j;
			}
		}

		// Sort the node list.
		i = 0;
		while (i < nodeCount - 1) {
			if (nodeX[i] <= nodeX[i + 1]) i++;
			else {
				swap = nodeX[i]; nodeX[i] = nodeX[i + 1]; nodeX[i + 1] = swap; if (i) i--;
			}
		}

		// Loop to draw one row of stripe segments.
		for (i = 0; i < nodeCount; i += 2) {

			// Rotate the points back to their original coordinate system.
			a = nodeX[i] * theCos - stripeY * theSin;
			b = stripeY * theCos + nodeX[i] * theSin;
			x = nodeX[i + 1] * theCos - stripeY * theSin;
			y = stripeY * theCos + nodeX[i + 1] * theSin;

			// Draw a single stripe segment.
			drawLineSegmentABtoXY(a, b, x, y);
		}
	}

	// Success.
	return true;
}


void renderClosureArea(const ClosureArea& area, float spacing) {
	glColor3f(1.0f, 0.0f, 0.0f);

	glBegin(GL_LINE_LOOP);// use this to draw the diagonal lines
	for (const LatLon& point : area.getPoints()) {
		glVertex2f(point.lon, point.lat);
	}
	glEnd();

	// Extract points from the ClosureArea object
	std::vector<LatLon> points = area.getPoints();
	long polygonCount = 1; // We're assuming one polygon for now
	long polygonCorners[1] = { points.size() }; // Number of corners in the polygon

	// Convert the points to the format expected by drawDiagonalStripes
	double** polygons = new double* [1];
	polygons[0] = new double[points.size() * 2];
	for (size_t i = 0; i < points.size(); ++i) {
		polygons[0][i * 2] = points[i].lon;
		polygons[0][i * 2 + 1] = points[i].lat;
	}

	// Draw the diagonal stripes
	drawDiagonalStripes(polygonCount, polygonCorners, polygons, spacing);

	// Clean up
	delete[] polygons[0];
	delete[] polygons;
}

