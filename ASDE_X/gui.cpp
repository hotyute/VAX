#include "main.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include <algorithm>

#include "gui.h"
#include "renderer.h"
#include "usermanager.h"
#include "tools.h"

std::vector<InterfaceFrame*> frames(256);
ChildFrame* focusChild = NULL, * lastFocus = NULL;
std::vector<InterfaceFrame*> deleteInterfaces, updateInterfaces;
bool updateLastFocus = false;
InterfaceFrame* _openedframe = NULL;

InterfaceFrame::InterfaceFrame(int index) {
	InterfaceFrame::children.resize(256);
	InterfaceFrame::interfaces.resize(NUM_SUB_INTERFACES);
	InterfaceFrame::index = index;
	InterfaceFrame::render = false;
	InterfaceFrame::renderAllInputText = false;
	InterfaceFrame::renderAllLabels = false;
}

InterfaceFrame::InterfaceFrame(int index, double width, double height) {
	InterfaceFrame::children.resize(256);
	InterfaceFrame::interfaces.resize(NUM_SUB_INTERFACES);
	InterfaceFrame::render = true;
	InterfaceFrame::renderAllInputText = false;
	InterfaceFrame::renderAllLabels = false;
	InterfaceFrame::index = index;
	BasicInterface* contentPane = new BasicInterface(0.0, width, 5.0, 0.0, height, 5.0, 0.0f, 0.0f, 0.0f, 0.6, false, false);
	contentPane->updateCoordinates();
	InterfaceFrame::interfaces[contentPane->index = CONTENT_PANE] = contentPane;
	BasicInterface* bounds = new BasicInterface(0.0, width, 5.0, 0.0, height, 5.0, 0.5f, 0.5f, 0.5f, 1.0, false, true);
	bounds->setBounds(true);
	bounds->updateCoordinates();
	InterfaceFrame::interfaces[bounds->index = FRAME_BOUNDS] = bounds;
}

InterfaceFrame::InterfaceFrame(int index, double x, double width, double y, double height) {
	InterfaceFrame::children.resize(256);
	InterfaceFrame::interfaces.resize(NUM_SUB_INTERFACES);
	InterfaceFrame::render = true;
	InterfaceFrame::renderAllInputText = false;
	InterfaceFrame::renderAllLabels = false;
	InterfaceFrame::index = index;
	BasicInterface* contentPane = new BasicInterface(x, width, 0.0, y, height, 0.0, 0.0f, 0.0f, 0.0f, 0.6, true, false);
	contentPane->updateCoordinates();
	InterfaceFrame::interfaces[contentPane->index = CONTENT_PANE] = contentPane;
	BasicInterface* bounds = new BasicInterface(x, width, 0.0, y, height, 0.0, 0.5f, 0.5f, 0.5f, 1.0, true, true);
	bounds->setBounds(true);
	bounds->updateCoordinates();
	InterfaceFrame::interfaces[bounds->index = FRAME_BOUNDS] = bounds;
}

InterfaceFrame::~InterfaceFrame()
{
	auto it = interfaces.begin();
	while (it != interfaces.end()) {
		BasicInterface* bi = (*it);
		it = interfaces.erase(it);
		delete bi;
	}

	auto it2 = children.begin();
	while (it2 != children.end()) {
		ChildFrame* cf = (*it2);
		it2 = children.erase(it2);
		delete cf;
	}
}

void InterfaceFrame::Pane1(double x, double width, double y, double height) {
	InterfaceFrame::render = true;
	BasicInterface* contentPane = new BasicInterface(x, width, -5.0, y, height, 5.0, 0.0f, 0.0f, 0.0f, 0.6, true, false);
	contentPane->updateCoordinates();
	InterfaceFrame::interfaces[contentPane->index = CONTENT_PANE] = contentPane;
	BasicInterface* bounds = new BasicInterface(x, width, -5.0, y, height, 5.0, 0.5f, 0.5f, 0.5f, 1.0, true, true);
	bounds->setBounds(true);
	bounds->updateCoordinates();
	InterfaceFrame::interfaces[bounds->index = FRAME_BOUNDS] = bounds;
}

void InterfaceFrame::UpdatePane1(double x, double width, double y, double height)
{
	BasicInterface& contentPane = *InterfaceFrame::interfaces[CONTENT_PANE];
	contentPane.setPosX(x), contentPane.setPosY(y);
	contentPane.setWidth(width), contentPane.setHeight(height);
	contentPane.updateCoordinates();

	BasicInterface& bounds = *InterfaceFrame::interfaces[FRAME_BOUNDS];
	bounds.setPosX(x), bounds.setPosY(y);
	bounds.setWidth(width), bounds.setHeight(height);
	bounds.updateCoordinates();
}

void InterfaceFrame::doOpen(bool multi_open, bool pannable)
{
	if (frames[InterfaceFrame::index] != this) {
		InterfaceFrame* frame = frames[InterfaceFrame::index];
		if (frame) {
			delete frame;
			frames[InterfaceFrame::index] = NULL;
		}
		frames[InterfaceFrame::index] = this;
	}
	InterfaceFrame::renderAllInputText = true;
	InterfaceFrame::render = true;
	InterfaceFrame::multi_open = multi_open;
	InterfaceFrame::pannable = pannable;
	renderInputTextFocus = true;
	renderInterfaces = true;
	renderDrawings = true;
	renderAllLabels = true;
	_openedframe = this;
	if (InterfaceFrame::index == CONNECT_INTERFACE)
		InterfaceFrame::children[CONN_CALLSIGN_LABEL]->setFocus();
	if (!multi_open)
		single_opened_frames++;
}

void InterfaceFrame::doClose()
{
	switch (InterfaceFrame::index)
	{
	case FP_INTERFACE:
	{
		opened_fp = NULL;
	}
	break;
	}
	InterfaceFrame::render = false;
	renderInterfaces = true;
	renderInputTextFocus = true;
	renderDrawings = true;
	renderFocus = true;
	_openedframe = NULL;
	if (!multi_open)
		single_opened_frames--;
}

/*void InterfaceFrame::addInputField(InputField* field) {
InterfaceFrame::inputFields.push_back(field);
}

std::vector<InputField*> InterfaceFrame::getInputFields() {
return InterfaceFrame::inputFields;
}*/

InputField::InputField(InterfaceFrame* frame, double width, double height) {
	InputField::frame = frame;
	InputField::p_protected = false;
	InputField::editable = true;
	InputField::centered = false;
	InputField::focus = false;
	InputField::type = INPUT_FIELD;
	InputField::inputTextDl = 0;
	BasicInterface* fieldBounds = new BasicInterface(0.0, width, 10.0, 0.0, height, 10.0, 1.0f, 1.0f, 1.0f, 0.8, false, true);
	fieldBounds->setBounds(true);
	fieldBounds->updateCoordinates();
	InputField::border = fieldBounds;
	InputField::child_interfaces.push_back(fieldBounds);
}

InputField::InputField(InterfaceFrame* frame, double x, double width, double padding_x, double y, double height, double padding_y) {
	InputField::frame = frame;
	InputField::p_protected = false;
	InputField::editable = true;
	InputField::centered = false;
	InputField::focus = false;
	InputField::type = INPUT_FIELD;
	InputField::inputTextDl = 0;
	BasicInterface* fieldBounds = new BasicInterface(x, width, padding_x, y, height, padding_y, 1.0f, 1.0f, 1.0f, 0.8, true, true);
	fieldBounds->setBounds(true);
	fieldBounds->updateCoordinates();
	InputField::border = fieldBounds;
	InputField::child_interfaces.push_back(fieldBounds);
}

InputField::~InputField()
{
}

void InputField::updatePos(double x, double width, double y, double height)
{
	InputField::border->setPosX(x), InputField::border->setPosY(y);
	InputField::border->setWidth(width), InputField::border->setHeight(height);
	InputField::border->updateCoordinates();
}

void InputField::doDrawing() {
}

void InputField::setFocus() {
	if (!InputField::focus) {
		if (focusChild != NULL) {
			focusChild->removeFocus();
			updateLastFocus = true;
		}
		InputField::pushInput(true, input_cursor);
		InputField::focus = true;
		focusChild = this;
		renderInputTextFocus = true;
	}
}

void InputField::removeFocus() {
	if (InputField::focus) {
		if (InputField::input.size() > 0) {
			InputField::popInput();
		}
		InputField::focus = false;
		lastFocus = this;
		focusChild = NULL;
		renderInputTextFocus = true;
	}
}

void InputField::doAction() {
}

void InputField::focusDrawing() {
}

void InputField::pushInput(bool uni, char c) {
	if (InputField::p_protected) {
		if (uni) {
			InputField::pp_input.push_back(c);
		}
		else {
			InputField::pp_input.push_back('*');
		}
	}
	InputField::input.push_back(c);
}

void InputField::popInput() {
	if (InputField::p_protected) {
		InputField::pp_input.pop_back();
	}
	InputField::input.pop_back();
}

void InputField::clearInput() {
	if (InputField::p_protected) {
		InputField::pp_input.clear();
	}
	InputField::input.clear();
}

void InputField::setInput(std::string text) {
	InputField::clearInput();
	InputField::input = text;
}

void InputField::setUneditable(std::string line)
{
	InputField::editable = false;
	InputField::centered = true;
	InputField::input = line;
}

void InputField::pass_characters(char* chars) {
	if (InputField::focus) {

		InputField::clearInput();
		while (*chars != '\0') {
			InputField::pushInput(false, *chars);
			++chars;
		}
		InputField::pushInput(true, input_cursor);
		renderInputTextFocus = true;
	}
}

bool InputField::can_type()
{
	BasicInterface& param = *InputField::border;
	double aW = param.getActualWidth();
	SelectObject(hDC, topBtnFont);
	TEXTMETRIC tm;
	GetTextMetrics(hDC, &tm);
	long ave = tm.tmAveCharWidth;
	int maxChars = aW / ave;
	if (InputField::p_protected)
	{
		if (InputField::pp_input.size() < (maxChars - 1))
			return true;
	}
	else
	{
		if (InputField::input.size() < (maxChars - 1))
			return true;
	}
	return false;
}


CloseButton::CloseButton(InterfaceFrame* frame, double width, double height) {
	CloseButton::frame = frame;
	CloseButton::focus = false;
	CloseButton::type = CLOSE_BUTTON;
	BasicInterface* inter = frame->interfaces[CONTENT_PANE];
	double startX = (inter->getStartX() + inter->getWidth()) - width;
	double startY = (inter->getStartY() + inter->getHeight()) - height;
	BasicInterface* closeBorder = new BasicInterface(startX, width, 0.0, startY, height, 0.0, 1.0f, 1.0f, 1.0f, 0.8, true, true);
	closeBorder->setBounds(true);
	closeBorder->updateCoordinates();
	CloseButton::border = closeBorder;
	CloseButton::child_interfaces.push_back(closeBorder);
}

void CloseButton::updatePos(double x, double width, double y, double height)
{
	BasicInterface* inter = CloseButton::frame->interfaces[CONTENT_PANE];
	double startX = (inter->getStartX() + inter->getWidth()) - width;
	double startY = (inter->getStartY() + inter->getHeight()) - height;

	CloseButton::border->setPosX(startX), CloseButton::border->setPosY(startY);
	CloseButton::border->setWidth(width), CloseButton::border->setHeight(height);
	CloseButton::border->updateCoordinates();
}

void CloseButton::doDrawing() {
	BasicInterface* inter11 = CloseButton::border;

	int startX = inter11->getStartX(), startY = inter11->getStartY();
	int endX = inter11->getEndX(), endY = inter11->getEndY();

	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	int line_width = 2;
	startX += line_width, startY += line_width;
	endX -= line_width, endY -= line_width;
	DrawVarLine(startX, endY, endX, startY, line_width, line_width);
	DrawVarLine(startX, startY, endX, endY, line_width, line_width);
}

void CloseButton::setFocus() {
	if (!CloseButton::focus) {
		if (focusChild != NULL) {
			focusChild->removeFocus();
			updateLastFocus = true;
		}
		CloseButton::focus = true;
		focusChild = this;
		renderFocus = true;
	}
}

void CloseButton::removeFocus() {
	if (CloseButton::focus) {
		CloseButton::focus = false;
		lastFocus = this;
		focusChild = NULL;
		renderFocus = true;
	}
}

void CloseButton::doAction() {
	CloseButton::frame->doClose();
}

void CloseButton::focusDrawing() {
}

ClickButton::ClickButton(InterfaceFrame* frame, std::string text, double x, double width, double y, double height) {
	ClickButton::frame = frame;
	ClickButton::focus = false;
	ClickButton::type = CLICK_BUTTON;
	ClickButton::text = text;
	BasicInterface* closeBounds = new BasicInterface(x, width, 0.0, y, height, 0.0, 1.0f, 1.0f, 1.0f, 0.8, true, true);
	closeBounds->setBounds(true);
	closeBounds->updateCoordinates();
	ClickButton::border = closeBounds;
	ClickButton::child_interfaces.push_back(closeBounds);
}

void ClickButton::doDrawing() {
	BasicInterface& param = *ClickButton::border;
	double x = (param.getStartX() + (param.getWidth() / 2));
	double y = (param.getStartY() + (param.getHeight() / 2));
	SelectObject(hDC, topBtnFont);
	SIZE extent = getTextExtent(ClickButton::text);
	TEXTMETRIC tm;
	GetTextMetrics(hDC, &tm);
	int tH = tm.tmAscent - tm.tmInternalLeading;
	int textXPos = x - (extent.cx / 2);
	int textYPos = y - (tH / 2);
	glColor4f(button_text_clr[0], button_text_clr[1], button_text_clr[2], 1.0f);
	glRasterPos2f(textXPos, textYPos);
	glPrint(ClickButton::text.c_str(), &topButtonBase);
}

void ClickButton::setFocus() {
	if (!ClickButton::focus) {
		if (focusChild != NULL) {
			focusChild->removeFocus();
			updateLastFocus = true;
		}
		ClickButton::focus = true;
		focusChild = this;
		renderFocus = true;
	}
}

void ClickButton::removeFocus() {
	if (ClickButton::focus) {
		//TODO Design

		ClickButton::focus = false;
		lastFocus = this;
		focusChild = NULL;
		renderFocus = true;
	}
}
void ClickButton::doAction() {
	switch (ClickButton::frame->index)
	{
	case CONNECT_INTERFACE://connect frame
	{
		switch (ClickButton::index)
		{
		case CONN_OKAY_BUTTON:
		{
			Identity& id = *USER->getIdentity();
			id.callsign = connect_callsign->input.c_str();
			id.login_name = connect_fullname->input.c_str();
			id.username = connect_username->input.c_str();
			id.password = connect_password->input.c_str();
			connect_closeb->doAction();
			connect();
		}
		break;
		case CONN_CANCEL_BUTTON:
		{
			connect_callsign->clearInput();
			connect_fullname->clearInput();
			connect_username->clearInput();
			connect_password->clearInput();
			connect_closeb->doAction();
		}
		break;
		}
	}
	break;
	}
}


void ClickButton::focusDrawing() {
}

void ClickButton::updatePos(double x, double width, double y, double height)
{
	ClickButton::border->setPosX(x), ClickButton::border->setPosY(y);
	ClickButton::border->setWidth(width), ClickButton::border->setHeight(height);
	ClickButton::border->updateCoordinates();
}

ComboBox::ComboBox(InterfaceFrame* frame, std::vector<std::string> options, double x, double width, double x_padding, double y, double height, double y_padding) {
	ComboBox::frame = frame;
	ComboBox::options = options;
	ComboBox::pos = 0;
	ComboBox::focus = false;
	ComboBox::type = COMBO_BOX;
	SelectObject(hDC, topBtnFont);
	ComboBox::largestExtent = getTextExtent(ComboBox::options[ComboBox::pos]);
	for (size_t i = 0; i < options.size(); i++) {
		ComboBox::extents.push_back(getTextExtent(ComboBox::options[i]));
		if (ComboBox::extents[i].cx > ComboBox::largestExtent.cx) {
			ComboBox::largestExtent = ComboBox::extents[i];
		}
	}
	int arrow_space = 10;
	BasicInterface* comboBounds = new BasicInterface(x, width == -1 ? ((ComboBox::largestExtent.cx + arrow_space) + width) : width, x_padding, y, height, y_padding, 1.0f, 1.0f, 1.0f, 0.8, true, true);
	comboBounds->setBounds(true);
	comboBounds->updateCoordinates();
	ComboBox::border = comboBounds;
	ComboBox::child_interfaces.push_back(comboBounds);
}

void ComboBox::updatePos(double x, double width, double y, double height) {
	SelectObject(hDC, topBtnFont);
	ComboBox::largestExtent = getTextExtent(ComboBox::options[ComboBox::pos]);
	for (size_t i = 0; i < options.size(); i++) {
		ComboBox::extents.push_back(getTextExtent(ComboBox::options[i]));
		if (ComboBox::extents[i].cx > ComboBox::largestExtent.cx) {
			ComboBox::largestExtent = ComboBox::extents[i];
		}
	}
	int arrow_space = 10;
	ComboBox::border->setPosX(x), ComboBox::border->setPosY(y);
	ComboBox::border->setWidth(width == -1 ? ((ComboBox::largestExtent.cx + arrow_space) + width) : width),
		ComboBox::border->setHeight(height);
	ComboBox::border->updateCoordinates();
}

void ComboBox::doDrawing() {
	BasicInterface& param = *ComboBox::border;
	double x = (param.getStartX() + (param.getActualWidth() / 2));
	double y = (param.getStartY() + (param.getActualHeight() / 2));
	SelectObject(hDC, topBtnFont);
	TEXTMETRIC tm;
	GetTextMetrics(hDC, &tm);
	int tH = tm.tmAscent - tm.tmInternalLeading;
	int textXPos = x - (ComboBox::extents[ComboBox::pos].cx / 2);
	int textYPos = y - (tH / 2);
	glColor4f(button_text_clr[0], button_text_clr[1], button_text_clr[2], 1.0f);
	glRasterPos2f(textXPos, textYPos);
	glPrint(ComboBox::options[ComboBox::pos].c_str(), &topButtonBase);
	int offsetX = 5, offsetY = 5;
	int length = 4;
	int line_size = 2;
	int startX = ((param.getStartX() + param.getWidth()) + offsetX), endX = ((param.getStartX() + param.getWidth()) + offsetX + length);
	int startY = (param.getStartY() + offsetY), startY2 = ((param.getStartY() + param.getHeight()) - offsetY), endY = (param.getStartY() + (param.getHeight() / 2));
	DrawVarLine(startX, startY2, endX, endY, line_size, line_size);
	DrawVarLine(startX, startY, endX, endY, line_size, line_size);

	startX = (param.getStartX() - offsetX), endX = (param.getStartX() - offsetX - length);
	DrawVarLine(startX, startY2, endX, endY, line_size, line_size);
	DrawVarLine(startX, startY, endX, endY, line_size, line_size);
}

void ComboBox::setFocus() {
	if (!ComboBox::focus) {
		if (focusChild != NULL) {
			focusChild->removeFocus();
			updateLastFocus = true;
		}
		ComboBox::focus = true;
		focusChild = this;
		renderFocus = true;
	}
}

void ComboBox::removeFocus() {
	if (ComboBox::focus) {
		//TODO Design

		ComboBox::focus = false;
		lastFocus = this;
		focusChild = NULL;
		renderFocus = true;
	}
}

void ComboBox::doAction() {
}

void ComboBox::focusDrawing() {
	if (ComboBox::focus) {
		int offsetX = 15, offsetY = 7;
		BasicInterface& param = *ComboBox::border;
		double x1 = (param.getStartX() - offsetX), x2 = (param.getStartX() + param.getActualWidth()) + offsetX;
		double y1 = (param.getStartY() - offsetY), y2 = (param.getStartY() + param.getActualHeight()) + offsetY;
		glColor3f(0.32549019607f, 0.03137254901f, 0.91372549019f);
		int line_size = 2;
		DrawVarLine(x1, y1, x1, y2, line_size, line_size);
		DrawVarLine(x1, y2, x2, y2, line_size, line_size);
		DrawVarLine(x2, y2, x2, y1, line_size, line_size);
		DrawVarLine(x2, y1, x1, y1, line_size, line_size);
	}
}

DisplayBox::DisplayBox(InterfaceFrame* frame, std::vector<ChatLine*> chat_lines, int numBlocks, double x, double width, double x_padding, double y, double height, double y_padding, bool centerText) {
	DisplayBox::frame = frame;
	DisplayBox::chat_lines = chat_lines;
	DisplayBox::centered = centerText;
	DisplayBox::numBlocks = numBlocks;
	DisplayBox::focus = false;
	DisplayBox::type = DISPLAY_BOX;
	BasicInterface* comboBounds = new BasicInterface(x, width, x_padding, y, height, y_padding, 1.0f, 1.0f, 1.0f, 0.8, true, true);
	comboBounds->setBounds(true);
	comboBounds->updateCoordinates();
	DisplayBox::border = comboBounds;
	DisplayBox::child_interfaces.push_back(comboBounds);
}

void DisplayBox::doDrawing() {
	BasicInterface& param = *DisplayBox::border;
	double x;
	double aW = param.getActualWidth();
	if (DisplayBox::centered) {
		x = (param.getStartX() + (aW / 2));
	}
	else {
		x = param.getStartX();
	}
	double y_height = (param.getActualHeight() / DisplayBox::numBlocks);
	double last_end_y = -1;
	SelectObject(hDC, topBtnFont);
	TEXTMETRIC tm;
	GetTextMetrics(hDC, &tm);
	long ave = tm.tmAveCharWidth;
	int maxChars = aW / ave;
	int noncp = 2;

	bool reset_idx = false;

	//split line
	auto it = DisplayBox::chat_lines.end();
	while (it != DisplayBox::chat_lines.begin()) {
		--it;
		ChatLine* m = *it;
		std::string text = m->getText();
		CHAT_TYPE type = m->getType();
		SIZE size = getTextExtent(text);
		if (size.cx > param.getActualWidth()) {
			std::vector<std::string> store;
			if (!wordWrap(store, text.c_str(), maxChars, 0)) {
				std::cout << "hello" << std::endl;
				continue;
			}

			std::string new_text;
			int remaining = 0;
			int s_size = store.size();

			for (int iy = 0; iy < s_size - 1; iy++) {
				new_text += store[iy];
				remaining++;
			}


			if (remaining > 0) {
				m->setText(rtrim(ltrim(store[s_size - 1])));
				ChatLine* c = new ChatLine(rtrim(ltrim(new_text)), type);
				it = DisplayBox::chat_lines.insert(it, c) + 1;


				c->split = m;
			}
		}
	}

	//unsplit lines
	auto i = DisplayBox::chat_lines.begin();
	while (i != DisplayBox::chat_lines.end()) {
		ChatLine* c = *i;

		bool set_pos = false;
		if (c->split) {
			ChatLine* n = *(i + 1);
			SIZE size = getTextExtent(c->getText() + n->getText());
			if (size.cx <= param.getActualWidth()) {
				c->setText(c->getText() + " " + n->getText());
				DisplayBox::chat_lines.erase(i + 1);
				c->split = n->split;
				delete n;
				reset_idx = true;
			}
			else
			{
				++i;
			}
		}
		else
		{
			++i;
		}
	}

	if (reset_idx) {
		resetReaderIdx();
		reset_idx = false;
	}

	//draw Text to screen
	for (size_t i = read_index; i < read_index + numBlocks; i++) {
		std::string text = DisplayBox::chat_lines[i]->getText();
		CHAT_TYPE type = DisplayBox::chat_lines[i]->getType();
		//std::cout << text << ", " << i << std::endl;
		double y, endY;
		if (last_end_y != -1) {
			y = (last_end_y - (y_height / 2));
			endY = (last_end_y - y_height);
		}
		else {
			y = (param.getEndY() - (y_height / 2));
			endY = (param.getEndY() - y_height);
		}
		SIZE size = getTextExtent(text);
		int tH = tm.tmAscent - tm.tmInternalLeading;
		double textXPos;
		if (DisplayBox::centered) {
			textXPos = x - (size.cx / 2);
		}
		else {
			textXPos = x + noncp;
		}
		double textYPos = y - (tH / 2);
		DisplayBox::SetChatTextColour(type);
		glRasterPos2f(textXPos, textYPos);
		glPrint(text.c_str(), &topButtonBase);
		last_end_y = endY;
	}

	glColor4f(button_text_clr[0], button_text_clr[1], button_text_clr[2], 1.0f);
	int offsetX = 3, offsetY = 0;
	int xLength = 8, yLength = 8;
	int line_size = 2;
	double startX = ((param.getStartX() + param.getActualWidth()) + offsetX), startX2 = ((param.getStartX() + param.getActualWidth()) + offsetX + (xLength / 2)), endX = ((param.getStartX() + param.getActualWidth()) + offsetX + xLength);
	double startY = (param.getStartY() + offsetY), endY = (param.getStartY() + offsetY + yLength);
	DrawVarLine(startX, endY, startX2, startY, line_size, line_size);
	DrawVarLine(startX2, startY, endX, endY, line_size, line_size);

	double startY3 = ((param.getStartY() + param.getActualHeight()) - offsetY), endY2 = ((param.getStartY() + param.getActualHeight()) - offsetY - yLength);
	DrawVarLine(startX, endY2, startX2, startY3, line_size, line_size);
	DrawVarLine(startX2, startY3, endX, endY2, line_size, line_size);

	double arrowLineOffset = 15;
	DrawVarLine(startX2, startY + arrowLineOffset, startX2, startY3 - arrowLineOffset, line_size, line_size);
}

void DisplayBox::setFocus() {
	if (!DisplayBox::focus) {
		if (focusChild != NULL) {
			focusChild->removeFocus();
			updateLastFocus = true;
		}
		DisplayBox::focus = true;
		focusChild = this;
		renderFocus = true;
	}
}

void DisplayBox::removeFocus() {
	if (DisplayBox::focus) {
		//TODO Design

		DisplayBox::focus = false;
		lastFocus = this;
		focusChild = NULL;
		renderFocus = true;
	}
}

void DisplayBox::doAction() {
}

void DisplayBox::focusDrawing() {
	if (DisplayBox::focus) {
	}
}

void DisplayBox::addLine(ChatLine* c) {
	if (DisplayBox::chat_lines.size() > max_history)
		DisplayBox::chat_lines.erase(DisplayBox::chat_lines.begin());
	DisplayBox::chat_lines.push_back(c);
	read_index++;
}

void DisplayBox::removeLine(ChatLine* c)
{
	auto it = std::find(std::begin(DisplayBox::chat_lines), std::end(DisplayBox::chat_lines), c);

	if (it != std::end(DisplayBox::chat_lines)) {
		DisplayBox::chat_lines.erase(it);
	}

	delete c;
}

void DisplayBox::updatePos(double x, double width, double y, double height)
{
	DisplayBox::border->setPosX(x), DisplayBox::border->setPosY(y);
	DisplayBox::border->setWidth(width), DisplayBox::border->setHeight(height);
	DisplayBox::border->updateCoordinates();
}




void DisplayBox::addLine(std::string text, CHAT_TYPE type) {
	ChatLine* c = new ChatLine(text, type);
	addLine(c);
}

void DisplayBox::doActionUp()
{
	if ((read_index - 1) >= 0) {
		--read_index;
		renderDrawings = true;
	}
}

void DisplayBox::doActionDown()
{
	if ((read_index + 1) <= ((chat_lines.end() - numBlocks) - chat_lines.begin())) {
		++read_index;
		renderDrawings = true;
	}
}

void DisplayBox::resetReaderIdx()
{
	read_index = ((chat_lines.end() - numBlocks) - chat_lines.begin());
}

void DisplayBox::SetChatTextColour(CHAT_TYPE t) {
	switch (t) {
	case CHAT_TYPE::MAIN:
	{
		glColor4f(button_text_clr[0], button_text_clr[1], button_text_clr[2], 1.0f);
	}
	break;
	case CHAT_TYPE::ERRORS:
	{
		glColor4f(text_error_clr[0], text_error_clr[1], text_error_clr[2], 1.0f);
	}
	break;
	case CHAT_TYPE::SYSTEM:
	{
		glColor4f(text_system_clr[0], text_system_clr[1], text_system_clr[2], 1.0f);
	}
	break;
	case CHAT_TYPE::ATC:
	{
		glColor4f(text_atc_clr[0], text_atc_clr[1], text_atc_clr[2], 1.0f);
	}
	break;
	default:
	{
		glColor4f(button_text_clr[0], button_text_clr[1], button_text_clr[2], 1.0f);
	}
	break;
	}
}

Label::Label(InterfaceFrame* interfaceFrame, std::string label, double width, double height)
{
	Label::frame = interfaceFrame;
	Label::labelTextDl = 0;
}

Label::Label(InterfaceFrame* interfaceFrame, std::string label, double x, double width, double padding_x, double y, double height, double padding_y)
{
	Label::frame = interfaceFrame;
	Label::input = label;
	Label::labelTextDl = 0;
	Label::focus = false;
	Label::type = LABEL_D;
	BasicInterface* fieldBounds = new BasicInterface(x, width, padding_x, y, height, padding_y, 1.0f, 1.0f, 1.0f, 0.8, true, true);
	fieldBounds->setRender(false);
	fieldBounds->setBounds(true);
	fieldBounds->updateCoordinates();
	Label::border = fieldBounds;
	Label::child_interfaces.push_back(fieldBounds);
}

void Label::updatePos(double x, double width, double y, double height)
{
	Label::border->setPosX(x), Label::border->setPosY(y);
	Label::border->setWidth(width), Label::border->setHeight(height);
	Label::border->updateCoordinates();
}

void Label::doDrawing()
{
}

void Label::setFocus()
{
}

void Label::removeFocus()
{
}

void Label::doAction() {

}

void Label::focusDrawing()
{
}

ChatLine::ChatLine(std::string line, CHAT_TYPE type)
{
	ChatLine::line = line;
	ChatLine::type = type;
}

ChatLine::~ChatLine()
{
}

void ChatLine::setType(CHAT_TYPE type)
{
	ChatLine::type = type;
}

CHAT_TYPE ChatLine::getType()
{
	return ChatLine::type;
}

void ChatLine::setText(std::string text)
{
	ChatLine::line = text;
}

std::string ChatLine::getText()
{
	return ChatLine::line;
}

void ChatLine::playChatSound()
{
	switch (ChatLine::type) {
	case CHAT_TYPE::MAIN:
	{
	}
	break;
	case CHAT_TYPE::ERRORS:
	{
		PlaySound(MAKEINTRESOURCE(IDW_SOUND2), NULL, SND_RESOURCE | SND_ASYNC);
	}
	break;
	case CHAT_TYPE::SYSTEM:
	{

	}
	break;
	case CHAT_TYPE::ATC:
	{
		PlaySound(MAKEINTRESOURCE(IDW_SOUND1), NULL, SND_RESOURCE | SND_ASYNC);
	}
	break;
	default:
	{
	}
	break;
	}
}

ChildFrame::~ChildFrame()
{
	auto it = child_interfaces.begin();
	while (it != child_interfaces.end()) {
		BasicInterface* bi = (*it);
		it = child_interfaces.erase(it);
		delete bi;
	}

	//no need to delete "border" as it's stored in child_interfaces
}

