#include <boost/date_time/posix_time/posix_time.hpp>
#include <thread>
#include <chrono>


#include "projection.h"
#include "renderer.h"
#include "point2d.h"
#include "topbutton.h"
#include "tools.h"
#include "gui.h"
#include "dxfdrawing.h"
#include "aircraft.h"
#include "events.h"
#include "flightplan.h"

std::unordered_map<std::string, Mirror*> mirrors_storage;
std::vector<Mirror*> mirrors;

bool renderAircraft = false, renderSector = false, renderButtons = false,
renderLegend = false, renderAllCallsigns = false, renderInterfaces = false,
renderInputText = false, renderConf = false, renderFocus = false, renderDrawings = false, what = false;

bool loadInterfaces = false;

bool resize = false;
int sectorDl, legendDl, buttonsDl, interfacesDl, confDl, aircraftDl, heavyDl, unkTarDl, focusDl, drawingDl;
unsigned int callSignBase, topButtonBase, confBase, legendBase, titleBase, labelBase, errorBase;
HFONT callSignFont = NULL, topBtnFont = NULL, confFont = NULL, legendFont = NULL, titleFont = NULL, labelFont = NULL,
errorFont = NULL;

void RenderChatInterface(ChatInterface&);
void RenderMirrorBorder(Mirror& mirror);
void RenderInputText(ChatInterface&, int&, std::string&, bool);
void RenderLabel(ChatInterface&, int&, std::string&, int);
void RenderFocuses();
void RenderDrawings();
int RenderAircraft(bool, int&);
int RenderUnknown(bool, int&);
int RenderCallsign(Aircraft&, bool, float, float);
void deleteFrame(InterfaceFrame*);

void set_projection(int clientWidth, int clientHeight, double c_lat, double c_lon, double zoom, bool top_bar);

const std::string* currentDateTime();

std::vector<GLdouble*> tesses;

float latitude = 0, longitude = 0;
double heading = 0;
double normMapScaleX;

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

	set_projection(width, height, mirror.getLat(), mirror.getLon(), zoom, false);

	glMatrixMode(GL_MODELVIEW);                            // Select The Modelview Matrix
	glLoadIdentity();                                    // Reset The Modelview Matrix

	SetView(mirror.getLat(), mirror.getLon());
}

void ResizeGLScene() {
	// Set up the viewport.
	int w_height = CLIENT_HEIGHT;
	if (w_height < FIXED_CLIENT_HEIGHT) {
		w_height = FIXED_CLIENT_HEIGHT;
	}
	BUTTON_HEIGHT = (BUTTON_LAYOUT_HEIGHT * w_height);

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
	
	set_projection(clientWidth, clientHeight, CENTER_LAT, CENTER_LON, mZoom, true);

	glMatrixMode(GL_MODELVIEW);                            // Select The Modelview Matrix
	glLoadIdentity();                                   // Reset The Modelview Matrix

	SetView(CENTER_LAT, CENTER_LON);
}

void SetView(double latitude, double longitude) {
	gluLookAt(longitude, latitude, 0, longitude, latitude, -1, 0, 1, 0);
}

void DrawGLScene() {
	glClear(GL_COLOR_BUFFER_BIT);
	if (DAY) {
		glClearColor(day_background[0], day_background[1], day_background[2], 0.0f);
	}
	else {
	}
	/*if (MOUSE_MOVE) {
	double lol[3];
	GetOGLPos(MOUSE_X, MOUSE_Y, lol);
	std::cout << std::fixed << lol[0] << ", " << lol[1] << std::endl;
	MOUSE_MOVE = false;
	}*/
	if (what && frames.size() >= 1) {
		std::vector<InterfaceFrame*>::iterator it = deleteInterfaces.begin();
		while (it != deleteInterfaces.end()) {
			deleteFrame((*it));
			it = deleteInterfaces.erase(it);
		}
		what = false;
	}
	if (renderSector) {
		glDeleteLists(sectorDl, 1);
		glPushMatrix();
		DrawSceneryData(nullptr);
		glPopMatrix();
		renderSector = false;
	}
	glCallList(sectorDl);
	if (renderAircraft) {
		//We use default zoom because they all get downsized by glScale anyway
		glDeleteLists(aircraftDl, 1);
		RenderAircraft(false, aircraftDl);
		glDeleteLists(heavyDl, 1);
		RenderAircraft(true, heavyDl);
		glDeleteLists(unkTarDl, 1);
		RenderUnknown(false, unkTarDl);
		renderAircraft = false;
	}
	if (AcfMap.size() > 0) {
		std::map<std::string, Aircraft*>::iterator iter;
		for (iter = AcfMap.begin(); iter != AcfMap.end(); iter++) {
			// iterator->first = key
			Aircraft* aircraft = iter->second;
			if (aircraft != NULL) {
				aircraft->lock();
				float acf_lat = aircraft->getLatitude();
				float acf_lon = aircraft->getLongitude();
				double acf_heading = aircraft->getHeading();
				bool renderCallsign = aircraft->getRenderCallsign();
				bool heavy = aircraft->isHeavy();
				bool standby = aircraft->getMode() == 0 ? true : false;
				double maxX = aircraftBlip->getMaxX();
				double maxY = aircraftBlip->getMaxY();
				aircraft->unlock();

				//move the aircraft first so we have proper movement along the map scale
				glPushMatrix();
				glTranslated((acf_lon - longitude), (acf_lat - latitude), 0.0f);

				//scale to what ever zoom we are using (DONT UPDATE GL COMPILE LIST WHILE DOING THIS, otherwise, it will go smaller)
				glTranslated(+longitude, +latitude, 0.0f);
				double zo = mZoom;
				double a_size = get_asize(heavy, standby, zo);
				glScaled(a_size, a_size, 1.0);
				glTranslated(-longitude, -latitude, 0.0f);

				//keep the aircraft drawing in correct aspect ratio
				glTranslated(+longitude, +latitude, 0.0f);
				glScaled(normMapScaleX, 1.0, 1.0);
				glTranslated(-longitude, -latitude, 0.0f);

				//set the aircraft heading - this needs to come after scaling aspect for proper rotation
				glTranslated(+longitude, +latitude, 0.0f);
				glRotatef(((float)acf_heading), 0.0f, 0.0f, -1.0f);
				glTranslated(-longitude, -latitude, 0.0f);
				if (standby) {
					glCallList(unkTarDl);
				}
				else {
					if (!heavy) {
						glCallList(aircraftDl);
					}
					else {
						glCallList(heavyDl);
					}
				}
				glPopMatrix();


				glPushMatrix();

				if (renderCallsign || renderAllCallsigns) {
					glDeleteLists(aircraft->Ccallsign, 1);
					RenderCallsign(*aircraft, heavy, latitude, longitude);
					aircraft->setRenderCallsign(false);
				}

				glTranslated((acf_lon - longitude), (acf_lat - latitude), 0.0f);

				//scale to what ever zoom we are using (DONT UPDATE GL COMPILE LIST WHILE DOING THIS, otherwise, it will go smaller)
				glTranslated(+longitude, +latitude, 0.0f);
				glScaled(a_size, a_size, 1.0);
				glTranslated(-longitude, -latitude, 0.0f);

				//keep the callsign drawing in correct aspect ratio
				glTranslated(+longitude, +latitude, 0.0f);
				glScaled(normMapScaleX, 1.0, 1.0);
				glTranslated(-longitude, -latitude, 0.0f);

				if (!standby) {
					glCallList(aircraft->Ccallsign);
				}
				glPopMatrix();
			}
		}
		if (renderAllCallsigns) {
			renderAllCallsigns = false;
		}
	}
}

void DrawInterfaces() {
	if (renderLegend) {
		glDeleteLists(legendDl, 1);
		RenderLegend();
		renderLegend = false;
	}
	glCallList(legendDl);
	if (renderButtons) {
		glDeleteLists(buttonsDl, 1);
		RenderButtons();
		renderButtons = false;
	}
	glCallList(buttonsDl);
	if (renderInterfaces) {
		if (loadInterfaces) {
			LoadInterfaces();
			loadInterfaces = false;
		}
		glDeleteLists(interfacesDl, 1);
		RenderInterfaces();
		renderInterfaces = false;
	}
	glCallList(interfacesDl);
	if (renderDrawings) {
		//TODO add optimizations (refresh per object basis)
		glDeleteLists(drawingDl, 1);
		RenderDrawings();
		renderDrawings = false;
	}
	glCallList(drawingDl);
	if (renderFocus) {
		//TODO add optimizations (refresh per object basis)
		glDeleteLists(focusDl, 1);
		RenderFocuses();
		renderFocus = false;
	}
	glCallList(focusDl);
	if (renderInputText) {
		if (updateLastFocus && lastFocus != NULL && lastFocus->type == INPUT_FIELD) {
			InputField* lastFocusField = (InputField*)lastFocus;
			glDeleteLists(lastFocusField->inputTextDl, 1);
			ChatInterface* last_border = lastFocus->child_interfaces[0];
			std::string l_input;
			if (lastFocusField->p_protected) {
				l_input = lastFocusField->pp_input;
			}
			else {
				l_input = lastFocusField->input;
			}
			RenderInputText(*last_border, lastFocusField->inputTextDl, l_input, lastFocusField->centered);
			updateLastFocus = false;
		}
		if (focusChild != NULL && focusChild->type == INPUT_FIELD) {
			InputField* focusField = (InputField*)focusChild;
			glDeleteLists(focusField->inputTextDl, 1);
			ChatInterface* border = focusField->border;
			std::string f_input;
			if (focusField->p_protected) {
				f_input = focusField->pp_input;
			}
			else {
				f_input = focusField->input;
			}
			RenderInputText(*border, focusField->inputTextDl, f_input, focusField->centered);
		}
		renderInputText = false;
	}
	for (InterfaceFrame* frame : frames) {
		if (frame && frame->render) {
			if (frame->renderAllInputText) {
				for (ChildFrame* child : frame->children) {
					if (child != NULL && child->type == INPUT_FIELD) {
						InputField* field = (InputField*)child;
						glDeleteLists(field->inputTextDl, 1);
						ChatInterface* border = field->border;
						std::string f_input;
						if (field->p_protected) {
							f_input = field->pp_input;
						}
						else {
							f_input = field->input;
						}
						RenderInputText(*border, field->inputTextDl, f_input, field->centered);
					}
				}
			}
			for (ChildFrame* child : frame->children) {
				if (child) {
					if (child->type == INPUT_FIELD) {
						InputField* field = (InputField*)child;
						glCallList(field->inputTextDl);
					}
				}
			}
		}
	}
	for (InterfaceFrame* frame : frames) {
		if (frame && frame->render) {
			for (ChildFrame* child : frame->children) {
				if (child != NULL && child->type == LABEL_D) {
					Label* label = (Label*)child;
					glDeleteLists(label->labelTextDl, 1);
					ChatInterface* border = label->border;
					RenderLabel(*border, label->labelTextDl, label->input, label->centered);
				}
			}
			for (ChildFrame* child : frame->children) {
				if (child) {
					if (child->type == LABEL_D) {
						Label* label = (Label*)child;
						glCallList(label->labelTextDl);
					}
				}
			}
		}
	}
	if (renderConf) {
		glDeleteLists(confDl, 1);
		RenderConf();
		renderConf = false;
	}
	glCallList(confDl);
}

void DrawMirrorScenes(Mirror& mirror)
{
	glClear(GL_COLOR_BUFFER_BIT);
	if (DAY) {
		glClearColor(day_background[0], day_background[1], day_background[2], 0.0f);
	}
	else
	{
	}


	glCallList(sectorDl);

	if (AcfMap.size() > 0) {
		std::map<std::string, Aircraft*>::iterator iter;
		for (iter = AcfMap.begin(); iter != AcfMap.end(); iter++) {
			// iterator->first = key
			Aircraft* aircraft = iter->second;
			if (aircraft != NULL) {
				aircraft->lock();
				float acf_lat = aircraft->getLatitude();
				float acf_lon = aircraft->getLongitude();
				double acf_heading = aircraft->getHeading();
				bool renderCallsign = aircraft->getRenderCallsign();
				bool heavy = aircraft->isHeavy();
				bool standby = aircraft->getMode() == 0 ? true : false;
				aircraft->unlock();


				//move the aircraft first so we have proper movement along the map scale
				glPushMatrix();
				glTranslated((acf_lon - longitude), (acf_lat - latitude), 0.0f);

				//set the size based on zoom
				glTranslated(+longitude, +latitude, 0.0f);
				double zo = mirror.getZoom();
				double a_size = get_asize(heavy, standby, zo);
				glScaled(a_size, a_size, 1.0);
				glTranslated(-longitude, -latitude, 0.0f);

				//keep the aircraft drawing in correct aspect ratio
				glTranslated(+longitude, +latitude, 0.0f);
				glScaled(normMapScaleX, 1.0, 1.0);
				glTranslated(-longitude, -latitude, 0.0f);

				//set the aircraft heading - this needs to come after scaling aspect for proper rotation
				glTranslated(+longitude, +latitude, 0.0f);
				glRotatef(((float)acf_heading), 0.0f, 0.0f, -1.0f);
				glTranslated(-longitude, -latitude, 0.0f);
				if (standby) {
					glCallList(unkTarDl);
				}
				else 
				{
					if (!heavy) {
						glCallList(aircraftDl);
					}
					else 
					{
						glCallList(heavyDl);
					}
				}
				glPopMatrix();


				glPushMatrix();

				glTranslated((acf_lon - longitude), (acf_lat - latitude), 0.0f);

				glTranslated(+longitude, +latitude, 0.0f);
				glScaled(a_size, a_size, 1.0);
				glTranslated(-longitude, -latitude, 0.0f);

				//keep the callsign drawing in correct aspect ratio
				glTranslated(+longitude, +latitude, 0.0f);
				glScaled(normMapScaleX, 1.0, 1.0);
				glTranslated(-longitude, -latitude, 0.0f);
				if (!standby) {
					glCallList(aircraft->Ccallsign);
				}
				glPopMatrix();
			}
		}
	}

	if (mirror.renderBorder) {
		glDeleteLists(mirror.borderDl, 1);
		RenderMirrorLines(mirror);
		mirror.renderBorder = false;
	}
	glCallList(mirror.borderDl);
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



int DrawSceneryData(Mirror* mirror) {
	sectorDl = glGenLists(1);

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

	glNewList(sectorDl, GL_COMPILE);
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
	for (PointTess* point2d1 : taxiways) {
		std::vector<LinearSegment*> coordinates11 = point2d1->get_coordinates();
		std::vector<LinearSegment*> holes11 = point2d1->get_holes();
		//std::cout << point2d1->get_coordinates()[0][0] << std::endl;
		glColor3f(taxiway_clr[0], taxiway_clr[1], taxiway_clr[2]);
		gluTessBeginPolygon(tess, 0);
		gluTessBeginContour(tess);
		for (LinearSegment* aCoordinates11 : coordinates11) {
			double* data = aCoordinates11->pt.as_array();
			gluTessVertex(tess, data, data);
		}
		gluTessEndContour(tess);
		gluTessEndPolygon(tess);
		if (DAY) {
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
		else {
			//handle NITE
		}
		//delete point2d;
	}
	for (PointTess* point2d1 : aprons) {
		std::vector<LinearSegment*> coordinates11 = point2d1->get_coordinates();
		std::vector<LinearSegment*> holes11 = point2d1->get_holes();
		//std::cout << point2d1->get_coordinates()[0][0] << std::endl;
		glColor3f(apron_clr[0], apron_clr[1], apron_clr[2]);
		gluTessBeginPolygon(tess, 0);
		gluTessBeginContour(tess);
		for (LinearSegment* aCoordinates11 : coordinates11) {
			double* data = aCoordinates11->pt.as_array();
			gluTessVertex(tess, data, data);
		}
		gluTessEndContour(tess);
		gluTessEndPolygon(tess);
		if (DAY) {
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
		else {
			//handle NITE
		}
		//delete point2d;
	}
	for (PointTess* point2d1 : runways) {
		std::vector<LinearSegment*> coordinates11 = point2d1->get_coordinates();
		std::vector<LinearSegment*> holes11 = point2d1->get_holes();
		//std::cout << point2d1->get_coordinates()[0][0] << std::endl;
		glColor3f(runway_clr[0], runway_clr[1], runway_clr[2]);
		gluTessBeginPolygon(tess, 0);
		gluTessBeginContour(tess);
		for (LinearSegment* aCoordinates11 : coordinates11) {
			double* data = aCoordinates11->pt.as_array();
			gluTessVertex(tess, data, data);
		}
		gluTessEndContour(tess);
		gluTessEndPolygon(tess);
		if (DAY) {
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
		else {
			//handle NITE
		}
		//delete point2d;
	}
	for (PointTess* point2d1 : parking) {
		std::vector<LinearSegment*> coordinates11 = point2d1->get_coordinates();
		std::vector<LinearSegment*> holes11 = point2d1->get_holes();
		//std::cout << point2d1->get_coordinates()[0][0] << std::endl;
		glColor3f(parking_clr[0], parking_clr[1], parking_clr[2]);
		gluTessBeginPolygon(tess, 0);                   // with NULL data
		gluTessBeginContour(tess);
		for (LinearSegment* aCoordinates11 : coordinates11) {
			double* data = aCoordinates11->pt.as_array();
			gluTessVertex(tess, data, data);
		}
		gluTessEndContour(tess);
		gluTessEndPolygon(tess);
		if (DAY) {
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
		else {
			//handle NITE
		}
	}
	glEndList();

	gluDeleteTess(tess);        // delete after tessellation
	deleteTess();
	return 1;
}

void RenderLegend() {
	legendDl = glGenLists(1);
	glNewList(legendDl, GL_COMPILE);
	std::string text1 = "Heavy Aircraft";

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, CLIENT_WIDTH, 0.0, CLIENT_HEIGHT);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(heavy_clr[0], heavy_clr[1], heavy_clr[2]);
	glRasterPos2f(18, 35);
	glPrint(text1.c_str(), &legendBase);	// Print GL Text To The Screen

	std::string text2 = "Regular Aircraft";

	glColor3f(regular_clr[0], regular_clr[1], regular_clr[2]);
	glRasterPos2f(18, 20);
	glPrint(text2.c_str(), &legendBase);	// Print GL Text To The Screen


	//Restore old ortho
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glEndList();
}

void RenderButtons() {
	int w_width = CLIENT_WIDTH;
	if (w_width < FIXED_CLIENT_WIDTH) {
		w_width = FIXED_CLIENT_WIDTH;
	}
	int w_height = CLIENT_HEIGHT;
	if (w_height < FIXED_CLIENT_HEIGHT) {
		w_height = FIXED_CLIENT_HEIGHT;
	}
	BUTTON_HEIGHT = (BUTTON_LAYOUT_HEIGHT * w_height);

	buttonsDl = glGenLists(1);

	glNewList(buttonsDl, GL_COMPILE);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, CLIENT_WIDTH, 0.0, CLIENT_HEIGHT);

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	//std::cout << viewport[0] << ", " << viewport[1] << ", " << viewport[2] << ", " <<  viewport[3] << ", " << std::endl;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(button_bg[0], button_bg[1], button_bg[2]);
	glBegin(GL_POLYGON);
	glVertex2d(0, CLIENT_HEIGHT);
	glVertex2d(0, (CLIENT_HEIGHT - BUTTON_HEIGHT));
	glVertex2d(CLIENT_WIDTH, (CLIENT_HEIGHT - BUTTON_HEIGHT));
	glVertex2d(CLIENT_WIDTH, CLIENT_HEIGHT);
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
		int width = (curButton.getWidth() * w_width);
		int height = BUTTON_HEIGHT;
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
			width = (CLIENT_WIDTH - x);
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
				glRasterPos2f(textXPos2, textYPos2);
				glPrint(text2.c_str(), &topButtonBase);

				int textYPos3 = (textYPos - tH) - (padding + linePadding);
				std::string text3 = curButton.getOption3();
				SIZE extent3 = getTextExtent(text3);
				int textXPos3 = (x + (width / 2)) - (extent3.cx / 2);
				glRasterPos2f(textXPos3, textYPos3);
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
					glColor4f(0.87843137254f, 0.67450980392f, 0.05490196078f, 1.0f);
					glRasterPos2f(textXPos2, textYPos2);
					glPrint(first2.c_str(), &topButtonBase);

					glColor4f(button_text_clr[0], button_text_clr[1], button_text_clr[2], 1.0f);
					glRasterPos2f((textXPos2 + first_e.cx), textYPos2);
					glPrint(divider.c_str(), &topButtonBase);

					glColor4f(button_text_clr[0], button_text_clr[1], button_text_clr[2], 1.0f);
					glRasterPos2f((textXPos2 + first_e.cx + center_e.cx), textYPos2);
					glPrint(second2.c_str(), &topButtonBase);
				}
				else {
					glRasterPos2f(textXPos2, textYPos2);
					glPrint(fullString2.c_str(), &topButtonBase);
				}
				textYPos += (padding + linePadding);
			}
			if (second.length() > 0) {
				SIZE center_e = getTextExtent(divider);
				SIZE first_e = getTextExtent(first);
				glColor4f(0.87843137254f, 0.67450980392f, 0.05490196078f, 1.0f);
				glRasterPos2f(textXPos, textYPos);
				glPrint(first.c_str(), &topButtonBase);

				glColor4f(button_text_clr[0], button_text_clr[1], button_text_clr[2], 1.0f);
				glRasterPos2f((textXPos + first_e.cx), textYPos);
				glPrint(divider.c_str(), &topButtonBase);

				glColor4f(button_text_clr[0], button_text_clr[1], button_text_clr[2], 1.0f);
				glRasterPos2f((textXPos + first_e.cx + center_e.cx), textYPos);
				glPrint(second.c_str(), &topButtonBase);
			}
			else {
				glColor4f(button_text_clr[0], button_text_clr[1], button_text_clr[2], 1.0f);
				glRasterPos2f(textXPos, textYPos);
				glPrint(fullString.c_str(), &topButtonBase);
			}
		}
		last_index = index;
	}
	//Restore old ortho
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glEndList();
}

void RenderInterfaces() {
	int w_width = CLIENT_WIDTH;
	if (w_width < FIXED_CLIENT_WIDTH) {
		w_width = FIXED_CLIENT_WIDTH;
	}
	int w_height = CLIENT_HEIGHT;
	if (w_height < FIXED_CLIENT_HEIGHT) {
		w_height = FIXED_CLIENT_HEIGHT;
	}

	interfacesDl = glGenLists(1);

	glNewList(interfacesDl, GL_COMPILE);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, CLIENT_WIDTH, 0.0, CLIENT_HEIGHT);

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	//std::cout << viewport[0] << ", " << viewport[1] << ", " << viewport[2] << ", " <<  viewport[3] << ", " << std::endl;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glLineWidth((GLfloat)1.0f);

	for (InterfaceFrame* frame : frames) {
		if (frame && frame->render) {
			int pos = 0;
			for (ChatInterface* inter1 : frame->interfaces) {
				RenderChatInterface(*inter1);
				if (pos == 0) {
					if (frame->title.size() > 0) {
						std::string title = frame->title;
						int offsetY = 10;
						int x = inter1->getStartX() + (inter1->getWidth() / 2);
						int y = inter1->getStartY() + (inter1->getHeight() - offsetY);
						SelectObject(hDC, titleFont);
						SIZE extent = getTextExtent(title);
						TEXTMETRIC tm;
						GetTextMetrics(hDC, &tm);
						int tH = tm.tmAscent - tm.tmInternalLeading;
						int textXPos = x - (extent.cx / 2);
						int textYPos = y - (tH / 2);
						glColor4f(button_text_clr[0], button_text_clr[1], button_text_clr[2], 1.0f);
						glRasterPos2f(textXPos, textYPos);
						glPrint(title.c_str(), &titleBase);

						//border line
						glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
						int b_offset_Y = 25;
						glBegin(GL_LINES);
						glVertex2f(inter1->getStartX() + inter1->getWidth(), inter1->getStartY() + (inter1->getHeight() - b_offset_Y));
						glVertex2f(inter1->getStartX(), inter1->getStartY() + (inter1->getHeight() - b_offset_Y));
						glEnd();
					}
				}
				pos++;
			}
			for (ChildFrame* children : frame->children) {
				if (children) {
					for (ChatInterface* inter2 : children->child_interfaces) {
						if (inter2->isRender()) {
							RenderChatInterface(*inter2);
						}
					}
				}
			}
		}
	}

	glLineWidth((GLfloat)1.0f);

	//Restore old ortho
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
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

void RenderDrawings() {
	int w_width = CLIENT_WIDTH;
	if (w_width < FIXED_CLIENT_WIDTH) {
		w_width = FIXED_CLIENT_WIDTH;
	}
	int w_height = CLIENT_HEIGHT;
	if (w_height < FIXED_CLIENT_HEIGHT) {
		w_height = FIXED_CLIENT_HEIGHT;
	}

	drawingDl = glGenLists(1);

	glNewList(drawingDl, GL_COMPILE);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, CLIENT_WIDTH, 0.0, CLIENT_HEIGHT);

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	//std::cout << viewport[0] << ", " << viewport[1] << ", " << viewport[2] << ", " <<  viewport[3] << ", " << std::endl;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	for (InterfaceFrame* frame : frames) {
		if (frame && frame->render) {
			for (ChildFrame* children : frame->children) {
				if (children) {
					children->doDrawing();
				}
			}
		}
	}
	//Restore old ortho
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glEndList();
}

void RenderChatInterface(ChatInterface& interface1) {
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
	GLdouble* vertex;
	int i;

	vertex = (GLdouble*)malloc(6 * sizeof(GLdouble));
	vertex[0] = coords[0];
	vertex[1] = coords[1];
	vertex[2] = coords[2];
	for (i = 2; i < 6; i++) {
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
	GLfloat winX, winY, winZ;

	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	winX = (float)x;
	winY = (float)viewport[3] - (float)y;

	glReadPixels((int)x, (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

	gluUnProject(winX, winY, winZ, modelview, projection, viewport, &output[0], &output[1], &output[2]);
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

SIZE getTextExtent(std::string& s) {
	std::wstring ws = s2ws(s);
	LPCWSTR wText2 = ws.c_str();
	SIZE extent;
	GetTextExtentPoint32(hDC, wText2, lstrlenW(wText2), &extent);
	return extent;
}

void LoadInterfaces() {

	//set main pane
	const int controller_list_width = 86;
	const int arrow_offset = 15;//arrows that come with display box
	InterfaceFrame* textBox = new InterfaceFrame(MAIN_CHAT_INTERFACE);
	int x = (CLIENT_WIDTH / 3);
	double width = 0.66666666667;
	textBox->Pane1(x, width, 0, 125);

	const int c_padding = 10, m_padding = 10;
	// 2 arrow offset's for both display boxes
	// if you want to make input box longer, remove an arrow offset
	const int width_offset = (controller_list_width + (arrow_offset + arrow_offset) + c_padding + m_padding);

	//controller list
	std::vector<ChatLine*> list;
	list.push_back(new ChatLine("--Delivery--", CHAT_TYPE::MAIN));
	list.push_back(new ChatLine("1A - MIA_DEL", CHAT_TYPE::MAIN));
	list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	DisplayBox* displayBox = new DisplayBox(textBox, list, 10, x, controller_list_width, 5, 5, 114, 5, true);

	//chatbox
	textBox->children[displayBox->index = MAIN_CONTROLLERS_BOX] = displayBox;
	std::vector<ChatLine*> list2;
	list2.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	list2.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	list2.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	list2.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	list2.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	list2.push_back(new ChatLine("[Performing Version Check..]", CHAT_TYPE::SYSTEM));

	main_chat = new DisplayBox(textBox, list2, 6, x + (controller_list_width + arrow_offset),
		(CLIENT_WIDTH * width) - width_offset, m_padding, 27, 87, 10, false);
	textBox->children[main_chat->index = MAIN_CHAT_MESSAGES] = main_chat;
	textField = new InputField(textBox, x + (controller_list_width + arrow_offset),
		(CLIENT_WIDTH * width) - width_offset, 10.0, 5, 20, 5);
	textBox->children[textField->index = MAIN_CHAT_INPUT] = textField;
	frames[MAIN_CHAT_INTERFACE] = textBox;
	textField->setFocus();
}

void RenderInputText(ChatInterface& border, int& inputTextDl, std::string& text, bool centered) {
	int x, y = border.getStartY() + 6;
	double aW = border.getActualWidth();
	if (centered) {
		x = (border.getStartX() + (aW / 2));
	}
	else {
		x = border.getStartX() + 3;
	}
	int w_width = CLIENT_WIDTH;
	if (w_width < FIXED_CLIENT_WIDTH) {
		w_width = FIXED_CLIENT_WIDTH;
	}
	int w_height = CLIENT_HEIGHT;
	if (w_height < FIXED_CLIENT_HEIGHT) {
		w_height = FIXED_CLIENT_HEIGHT;
	}

	inputTextDl = glGenLists(1);

	glNewList(inputTextDl, GL_COMPILE);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, CLIENT_WIDTH, 0.0, CLIENT_HEIGHT);

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	SelectObject(hDC, topBtnFont);
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
	glPrint(finalTxt.c_str(), &topButtonBase);

	//Restore old ortho
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glEndList();
}

void RenderLabel(ChatInterface& border, int& labelDl, std::string& text, int centered) {
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
	int w_width = CLIENT_WIDTH;
	if (w_width < FIXED_CLIENT_WIDTH) {
		w_width = FIXED_CLIENT_WIDTH;
	}
	int w_height = CLIENT_HEIGHT;
	if (w_height < FIXED_CLIENT_HEIGHT) {
		w_height = FIXED_CLIENT_HEIGHT;
	}

	labelDl = glGenLists(1);

	glNewList(labelDl, GL_COMPILE);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, CLIENT_WIDTH, 0.0, CLIENT_HEIGHT);

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

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

	//Restore old ortho
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glEndList();
}

void RenderConf() {
	int w_width = CLIENT_WIDTH;
	if (w_width < FIXED_CLIENT_WIDTH) {
		w_width = FIXED_CLIENT_WIDTH;
	}
	int w_height = CLIENT_HEIGHT;
	if (w_height < FIXED_CLIENT_HEIGHT) {
		w_height = FIXED_CLIENT_HEIGHT;
	}

	confDl = glGenLists(1);

	glNewList(confDl, GL_COMPILE);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, CLIENT_WIDTH, 0.0, CLIENT_HEIGHT);

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor4f(conf_clr[0], conf_clr[1], conf_clr[2], 1.0f);
	std::string config = "RWY CONFIG: 26L27-00";
	glRasterPos2f(30, (CLIENT_HEIGHT - (CLIENT_HEIGHT / 6)));
	glPrint(config.c_str(), &confBase);

	const std::string* date1 = currentDateTime();
	glRasterPos2f(50, (CLIENT_HEIGHT / 6));
	glPrint(date1[0].c_str(), &confBase);

	//std::string date2 = "1645/22";
	glRasterPos2f(53, (CLIENT_HEIGHT / 6) - 12);
	glPrint(date1[1].c_str(), &confBase);


	//Restore old ortho
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glEndList();
}

int RenderAircraft(bool heavy, int& acf_base) {
	double aircraft_size = get_default_asize(heavy, false);
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

	glTranslatef(+longitude, +latitude, 0.0f);
	glRotatef(((float)heading), 0.0f, 0.0f, -1.0f);
	glTranslatef(-longitude, -latitude, 0.0f);
	glScaled(aircraft_size, aircraft_size, 1);
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
	double aircraft_size = get_default_asize(heavy, false);
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

	glTranslatef(+longitude, +latitude, 0.0f);
	glRotatef(((float)heading), 0.0f, 0.0f, -1.0f);
	glTranslatef(-longitude, -latitude, 0.0f);
	glScaled(aircraft_size, aircraft_size, 1);
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
	double aircraft_size = get_default_asize(heavy, false);
	double minX = aircraftBlip->getMinX();
	double minY = aircraftBlip->getMinY();
	double maxX = aircraftBlip->getMaxX();
	double maxY = aircraftBlip->getMaxY();

	double offsetX = maxX;
	double offsetY = maxY;
	float vMaxX = (longitude + (offsetX * aircraft_size));
	float vMaxY = (latitude + (offsetY * aircraft_size));

	std::cout << aircraft_size << ", " << u_aircraft_size << std::endl;

	//Draw Callsign
	glNewList(aircraft.Ccallsign, GL_COMPILE);

	glColor4f(callsign_clr[0], callsign_clr[1], callsign_clr[2], 1.0f);
	glRasterPos2f(vMaxX, vMaxY); // set position
	glPrint(aircraft.getCallsign().c_str(), &callSignBase);

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

void RenderFocuses() {
	int w_width = CLIENT_WIDTH;
	if (w_width < FIXED_CLIENT_WIDTH) {
		w_width = FIXED_CLIENT_WIDTH;
	}
	int w_height = CLIENT_HEIGHT;
	if (w_height < FIXED_CLIENT_HEIGHT) {
		w_height = FIXED_CLIENT_HEIGHT;
	}

	focusDl = glGenLists(1);

	glNewList(focusDl, GL_COMPILE);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, CLIENT_WIDTH, 0.0, CLIENT_HEIGHT);

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	for (InterfaceFrame* frame : frames) {
		if (frame && frame->render) {
			for (ChildFrame* children : frame->children) {
				if (children) {
					children->focusDrawing();
				}
			}
		}
	}
	//Restore old ortho
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glEndList();
}

void deleteFrame(InterfaceFrame* frame) {
	std::vector<InterfaceFrame*>::iterator it = std::remove(frames.begin(), frames.end(), frame);
	frames.erase(it, frames.end());
}

void set_projection(int clientWidth, int clientHeight, double c_lat, double c_lon, double zoom, bool top_bar) {

	// Clamp the zoom.
	if (zoom > MAX_ZOOM) {
		zoom = MAX_ZOOM;
	}
	else if (zoom < MIN_ZOOM) {
		zoom = MIN_ZOOM;
	}

	// Set the min/max lat/lon based on center point, zoom, and client aspect.
	double coordSysMinLon = c_lon - ((clientWidth / 2.0) * (zoom / NM_PER_DEG));
	double coordSysMaxLon = c_lon + ((clientWidth / 2.0) * (zoom / NM_PER_DEG));
	double coordSysMinLat = c_lat - (((top_bar ? (clientHeight + BUTTON_HEIGHT) : clientHeight) / 2.0) * (zoom / NM_PER_DEG));
	double coordSysMaxLat = c_lat + (((top_bar ? (clientHeight + BUTTON_HEIGHT) : clientHeight) / 2.0) * (zoom / NM_PER_DEG));

	// Calculate coordinate system boundaries.
	double coordSysWidth = coordSysMaxLon - coordSysMinLon;
	double coordSysHeight = coordSysMaxLat - coordSysMinLat;
	double minX = 0.0 - (coordSysWidth / 2.0);
	double maxX = coordSysWidth / 2.0;
	double minY = 0.0 - (coordSysHeight / 2.0);
	double maxY = coordSysHeight / 2.0;

	//System.out.println(coordSysMinLon + ", " + coordSysMaxLon);

	// Set up the projection matrix based on these boundaries.
	gluOrtho2D(minX, maxX, minY, maxY);

	double nmPerLon = 54.0;
	double mapScaleX = (nmPerLon / 60.0);
	normMapScaleX = 1.0 / mapScaleX;
	glScaled(mapScaleX, 1.0, 1.0);
}