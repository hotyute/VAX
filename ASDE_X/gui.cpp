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
	InterfaceFrame::index = index;
	InterfaceFrame::render = false;
	InterfaceFrame::renderAllInputText = false;
	InterfaceFrame::renderAllLabels = false;
}

InterfaceFrame::InterfaceFrame(int index, double width, double height) {
	InterfaceFrame::children.resize(256);
	InterfaceFrame::render = true;
	InterfaceFrame::renderAllInputText = false;
	InterfaceFrame::renderAllLabels = false;
	InterfaceFrame::index = index;
	BasicInterface* contentPane = new BasicInterface(0.0, width, 5.0, 0.0, height, 5.0, 0.0f, 0.0f, 0.0f, 0.6, false, false);
	contentPane->updateCoordinates();
	InterfaceFrame::interfaces.push_back(contentPane);
	BasicInterface* bounds = new BasicInterface(0.0, width, 5.0, 0.0, height, 5.0, 0.5f, 0.5f, 0.5f, 1.0, false, true);
	bounds->setBounds(true);
	bounds->updateCoordinates();
	InterfaceFrame::interfaces.push_back(bounds);
}

InterfaceFrame::InterfaceFrame(int index, double x, double width, double y, double height) {
	InterfaceFrame::children.resize(256);
	InterfaceFrame::render = true;
	InterfaceFrame::renderAllInputText = false;
	InterfaceFrame::renderAllLabels = false;
	InterfaceFrame::index = index;
	BasicInterface* contentPane = new BasicInterface(x, width, 0.0, y, height, 0.0, 0.0f, 0.0f, 0.0f, 0.6, true, false);
	contentPane->updateCoordinates();
	interfaces.push_back(contentPane);
	BasicInterface* bounds = new BasicInterface(x, width, 0.0, y, height, 0.0, 0.5f, 0.5f, 0.5f, 1.0, true, true);
	bounds->setBounds(true);
	bounds->updateCoordinates();
	interfaces.push_back(bounds);
}

void InterfaceFrame::Pane1(double x, double width, double y, double height) {
	InterfaceFrame::render = true;
	BasicInterface* contentPane = new BasicInterface(x, width, -5.0, y, height, 5.0, 0.0f, 0.0f, 0.0f, 0.6, true, false);
	contentPane->updateCoordinates();
	InterfaceFrame::interfaces.push_back(contentPane);
	BasicInterface* bounds = new BasicInterface(x, width, -5.0, y, height, 5.0, 0.5f, 0.5f, 0.5f, 1.0, true, true);
	bounds->setBounds(true);
	bounds->updateCoordinates();
	InterfaceFrame::interfaces.push_back(bounds);
}

void InterfaceFrame::doOpen(int index, bool multi_open, bool pannable)
{
	if (frames[index] != this) {
		InterfaceFrame *frame = frames[index];
		if (frame) {
			delete frame;
			frames[index] = NULL;
		}
		frames[index] = this;
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
	if (index == CONNECT_INTERFACE)
		InterfaceFrame::children[CONN_CALLSIGN_LABEL]->setFocus();
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


CloseButton::CloseButton(InterfaceFrame* frame, double width, double height) {
	CloseButton::frame = frame;
	CloseButton::focus = false;
	CloseButton::type = CLOSE_BUTTON;
	BasicInterface* inter = frame->interfaces[0];
	double startX = (inter->getStartX() + inter->getWidth()) - width;
	double startY = (inter->getStartY() + inter->getHeight()) - height;
	BasicInterface* closeBorder = new BasicInterface(startX, width, 0.0, startY, height, 0.0, 1.0f, 1.0f, 1.0f, 0.8, true, true);
	closeBorder->setBounds(true);
	closeBorder->updateCoordinates();
	CloseButton::border = closeBorder;
	CloseButton::child_interfaces.push_back(closeBorder);
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
			Identity &id = *USER->getIdentity();
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
	BasicInterface* comboBounds = new BasicInterface(x, ((ComboBox::largestExtent.cx + arrow_space) + width), x_padding, y, height, y_padding, 1.0f, 1.0f, 1.0f, 0.8, true, true);
	comboBounds->setBounds(true);
	comboBounds->updateCoordinates();
	ComboBox::border = comboBounds;
	ComboBox::child_interfaces.push_back(comboBounds);
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
	DisplayBox::chat_line_history.insert(DisplayBox::chat_line_history.end(), chat_lines.begin(), chat_lines.end());
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
	for (size_t i = 0; i < DisplayBox::chat_lines.size(); i++) {
		std::string text = DisplayBox::chat_lines[i]->getText();
		SIZE size = getTextExtent(text);
		if (size.cx > param.getActualWidth()) {
			std::vector<std::string> store;
			wordWrap(store, text.c_str(), maxChars, 0);
			int i3;
			int s_size = store.size();
			int timesShifted = 0;
			while (timesShifted < (s_size - 1)) {
				std::string temp = DisplayBox::chat_lines[0]->getText();
				for (i3 = 0; i3 < i; i3++) {
					DisplayBox::chat_lines[i3] = DisplayBox::chat_lines[i3 + 1];
				}
				DisplayBox::chat_lines[i]->setText(temp);
				timesShifted++;
			}
			for (size_t i2 = 0; i2 < s_size; i2++) {
				std::string s_text = store[i2];
				DisplayBox::chat_lines[i - ((s_size - 1) - i2)]->setText(ltrim(s_text));
			}
		}
	}
	for (size_t i = 0; i < DisplayBox::chat_lines.size(); i++) {
		std::string text = DisplayBox::chat_lines[i]->getText();
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
		glColor4f(button_text_clr[0], button_text_clr[1], button_text_clr[2], 1.0f);
		glRasterPos2f(textXPos, textYPos);
		glPrint(text.c_str(), &topButtonBase);
		last_end_y = endY;
	}
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

void DisplayBox::display_pos()
{
	auto it = chat_line_history.begin() + read_index;
	int i = 0;
	while (it != chat_line_history.end()) {
		if (i >= numBlocks)
			break;
		DisplayBox::chat_lines[i++] = *it;
		it++;
	}
}



void DisplayBox::addLine(std::string text, CHAT_TYPE type) {
	DisplayBox::chat_lines.erase(DisplayBox::chat_lines.begin());
	ChatLine* c = new ChatLine(text, type);
	DisplayBox::chat_lines.push_back(c);
	if (chat_line_history.size() >= max_history)
	{
		ChatLine* first_line = chat_line_history.front();
		DisplayBox::chat_line_history.erase(DisplayBox::chat_line_history.begin());
		delete first_line;
	}
	read_index++;
	DisplayBox::chat_line_history.push_back(c);
}

void DisplayBox::doActionUp()
{
	--read_index;
	renderDrawings = true;
	display_pos();
}

void DisplayBox::doActionDown()
{
	++read_index;
	renderDrawings = true;
	display_pos();
}

void DisplayBox::resetReaderIdx()
{
	if (read_index != (chat_line_history.end() - numBlocks) - chat_line_history.begin()) {
		auto it = chat_line_history.end() - numBlocks;
		int i = 0;
		read_index = (it - chat_line_history.begin());
		while (it != chat_line_history.end()) {
			DisplayBox::chat_lines[i++] = *it;
			it++;
		}
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
