#include "main.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include <algorithm>

#include "gui.h"
#include "renderer.h"
#include "usermanager.h"
#include "tools.h"

std::vector<InterfaceFrame*> frames_def(700);
std::vector<InterfaceFrame*> rendered_frames;
ChildFrame* focusChild = nullptr, * lastFocus = nullptr;
std::vector<InterfaceFrame*> deleteInterfaces, updateInterfaces;
InterfaceFrame* _openedframe = nullptr;

InterfaceFrame::InterfaceFrame(int id) {
	InterfaceFrame::children.resize(256);
	InterfaceFrame::interfaces.resize(NUM_SUB_INTERFACES);
	InterfaceFrame::id = id;
	InterfaceFrame::render = false;
	InterfaceFrame::renderAllInputText = false;
	InterfaceFrame::renderAllLabels = false;
}

InterfaceFrame::InterfaceFrame(int id, double width, double height) {
	InterfaceFrame::children.resize(256);
	InterfaceFrame::interfaces.resize(NUM_SUB_INTERFACES);
	InterfaceFrame::render = true;
	InterfaceFrame::renderAllInputText = false;
	InterfaceFrame::renderAllLabels = false;
	InterfaceFrame::id = id;
	BasicInterface* contentPane = new BasicInterface(0.0, width, 5.0, 0.0, height, 5.0, 0.0f, 0.0f, 0.0f, 0.6, false, false);
	contentPane->updateCoordinates();
	InterfaceFrame::interfaces[contentPane->index = CONTENT_PANE] = contentPane;
	BasicInterface* bounds = new BasicInterface(0.0, width, 5.0, 0.0, height, 5.0, 0.5f, 0.5f, 0.5f, 1.0, false, true);
	bounds->setBounds(true);
	bounds->updateCoordinates();
	InterfaceFrame::border = bounds;
	InterfaceFrame::interfaces[bounds->index = FRAME_BOUNDS] = bounds;
}

InterfaceFrame::InterfaceFrame(int id, double x, double width, double y, double height) {
	InterfaceFrame::children.resize(256);
	InterfaceFrame::interfaces.resize(NUM_SUB_INTERFACES);
	InterfaceFrame::render = true;
	InterfaceFrame::renderAllInputText = false;
	InterfaceFrame::renderAllLabels = false;
	InterfaceFrame::id = id;
	BasicInterface* contentPane = new BasicInterface(x, width, 0.0, y, height, 0.0, 0.0f, 0.0f, 0.0f, 0.6, true, false);
	contentPane->updateCoordinates();
	InterfaceFrame::interfaces[contentPane->index = CONTENT_PANE] = contentPane;
	BasicInterface* bounds = new BasicInterface(x, width, 0.0, y, height, 0.0, 0.5f, 0.5f, 0.5f, 1.0, true, true);
	bounds->setBounds(true);
	bounds->updateCoordinates();
	InterfaceFrame::border = bounds;
	InterfaceFrame::interfaces[bounds->index = FRAME_BOUNDS] = bounds;
}

InterfaceFrame::~InterfaceFrame()
{
	delete cur_pt;
	cur_pt = nullptr;
	delete s_pt;
	s_pt = nullptr;
	delete end_pt;
	end_pt = nullptr;

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

	//frames[this->index] = NULL;
}

void InterfaceFrame::Pane1(double x, double width, double y, double height) {
	InterfaceFrame::render = true;
	BasicInterface* contentPane = new BasicInterface(x, width, -5.0, y, height, 5.0, 0.0f, 0.0f, 0.0f, 0.6, true, false);
	contentPane->updateCoordinates();
	InterfaceFrame::interfaces[contentPane->index = CONTENT_PANE] = contentPane;
	BasicInterface* bounds = new BasicInterface(x, width, -5.0, y, height, 5.0, 0.5f, 0.5f, 0.5f, 1.0, true, true);
	bounds->setBounds(true);
	bounds->updateCoordinates();
	InterfaceFrame::border = bounds;
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
	doReplace();
	InterfaceFrame::renderAllInputText = true;
	InterfaceFrame::render = true;
	InterfaceFrame::multi_open = multi_open;
	InterfaceFrame::pannable = pannable;
	RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
	renderInterfaces = true;
	renderDrawings = true;
	renderAllLabels = true;
	_openedframe = this;
	if (InterfaceFrame::id == CONNECT_INTERFACE)
		InterfaceFrame::children[CONN_CALLSIGN_LABEL]->setFocus();
	if (!(InterfaceFrame::id == FP_INTERFACE && opened_fp))
	{
		if (!multi_open)
			single_opened_frames++;
	}
}

void InterfaceFrame::doClose()
{
	switch (this->id)
	{
	case FP_INTERFACE:
	{
		opened_fp = nullptr;
		main_chat_input->setFocus();
		break;
	}
	case CONNECT_INTERFACE:
	{
		main_chat_input->setFocus();
		break;
	}
	case CONTROLLER_INTERFACE:
	{
		main_chat_input->setFocus();
		break;
	}
	}
	InterfaceFrame::render = false;
	renderInterfaces = true;
	RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
	renderDrawings = true;
	renderFocus = true;
	_openedframe = nullptr;
	if (!multi_open)
		single_opened_frames--;
}

void InterfaceFrame::doReplace()
{
	InterfaceFrame* frame = frames_def[this->id];
	if (frame && frame != this) {
		if (focusChild && focusChild->getFrame() == frame)
		{
			focusChild->removeFocus();
			focusChild = nullptr;
		}
		frames_def[this->id] = this;
		rendered_frames[this->index = frame->index] = this;
		delete frame;
	}
}

void InterfaceFrame::doDefinition()
{
	InterfaceFrame* frame = frames_def[this->id];
	if (frame && frame != this)
	{
		frames_def[this->id] = this;
		delete frame;
	}
	else
	{
		frames_def[this->id] = this;
	}
}

void InterfaceFrame::doInsert()
{
	InterfaceFrame* frame = frames_def[this->id];
	if (frame)
	{
		if (frame != this)
			doReplace();
	}
	else
	{
		frames_def[this->id] = this;
		rendered_frames.push_back(this);
		this->index = rendered_frames.size() - 1;
	}
}

void InterfaceFrame::move(int dx, int dy)
{
	if (dx != 0 || dy != 0)
	{
		if (this->pannable) {
			for (BasicInterface* inter1 : this->interfaces) {
				if (inter1)
				{
					inter1->setPosX(inter1->getPosX() + dx);
					inter1->setPosY(inter1->getPosY() + dy);
					inter1->updateCoordinates();
				}
			}
			for (ChildFrame* children : this->children) {
				if (children) {
					for (BasicInterface* inter2 : children->child_interfaces) {
						inter2->setPosX(inter2->getPosX() + dx);
						inter2->setPosY(inter2->getPosY() + dy);
						inter2->updateCoordinates();
					}
				}
			}

			this->renderAllInputText = true;
			this->renderAllLabels = true;
			renderDrawings = true;
			renderFocus = true;
			renderInterfaces = true;
		}
	}
}

bool InterfaceFrame::withinClient()
{
	int s_x = border->getStartX(), e_x = border->getEndX();
	int s_y = border->getStartY(), e_y = border->getEndY();
	int dx = 0, dy = 0;
	if (e_x > CLIENT_WIDTH)
	{
		dx = CLIENT_WIDTH - e_x;
	}
	if (e_y > CLIENT_HEIGHT)
	{
		dy = CLIENT_HEIGHT - e_y;
	}

	printf("%d, %d\n", dx, dy);

	if (dx != 0 || dy != 0)
	{
		move(dx, dy);
		return false;
	}
	return true;
}

/*void InterfaceFrame::addInputField(InputField* field) {
InterfaceFrame::inputFields.push_back(field);
}

std::vector<InputField*> InterfaceFrame::getInputFields() {
return InterfaceFrame::inputFields;
}*/

InputField::InputField(InterfaceFrame* frame, double width, double height) {
	InputField::frame = frame;
	InputField::font = &topBtnFont;
	InputField::base = &topButtonBase;
	InputField::p_protected = false;
	InputField::editable = true;
	InputField::numbers = false;
	InputField::centered = false;
	InputField::focus = false;
	InputField::type = CHILD_TYPE::INPUT_FIELD;
	InputField::inputTextDl = 0;
	BasicInterface* fieldBounds = new BasicInterface(0.0, width, 10.0, 0.0, height, 10.0, 1.0f, 1.0f, 1.0f, 0.8, false, true);
	fieldBounds->setBounds(true);
	fieldBounds->updateCoordinates();
	InputField::border = fieldBounds;
	InputField::child_interfaces.push_back(fieldBounds);
}

InputField::InputField(InterfaceFrame* frame, double x, double width, double padding_x, double y, double height, double padding_y) {
	InputField::frame = frame;
	InputField::font = &topBtnFont;
	InputField::base = &topButtonBase;
	InputField::focus = false;
	InputField::type = CHILD_TYPE::INPUT_FIELD;
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
		if (focusChild) {
			focusChild->removeFocus();
			RenderChild(lastFocus);
		}
		InputField::setCursorAtEnd();
		InputField::focus = true;
		focusChild = this;
		RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
	}
}

void InputField::removeFocus() {
	if (InputField::focus)
	{
		removeCursor();
		InputField::focus = false;
		lastFocus = this;
		focusChild = nullptr;
		RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
		RenderChild(lastFocus);
		if (line_ptr)
		{
			handleBox();//this has to come last as it deletes the inputfield
		}
	}
}

void InputField::doAction() {
}

void InputField::focusDrawing() {
}

int InputField::handleClick(ChildFrame* clicked, int x, int y)
{
	return 0;
}

void InputField::pushInput(bool uni, char c) {
	if (numbers && !isdigit(c))
		return;
	if (max_chars != 0)
	{
		if ((input.size() + 1) > max_chars)
			return;
	}
	bool ins = cursor_pos < input.size() ? true : false;
	if (InputField::p_protected)
	{
		if (uni) {
			ins ? InputField::pp_input.insert(InputField::pp_input.begin() + cursor_pos, caps ? toupper(c) : c) : InputField::pp_input.push_back(caps ? toupper(c) : c);
		}
		else {
			ins ? InputField::pp_input.insert(InputField::pp_input.begin() + cursor_pos, '*') : InputField::pp_input.push_back('*');
		}
	}
	ins ? InputField::input.insert(InputField::input.begin() + cursor_pos, caps ? toupper(c) : c) : InputField::input.push_back(caps ? toupper(c) : c);
	InputField::cursor_input.push_back(' ');
	last_cursor_pos = cursor_pos;
	cursor_pos++;
}

void InputField::setCursorAtStart() {
	last_cursor_pos = cursor_pos = 0;
	setCursor();
}

void InputField::setCursorAtEnd() {
	last_cursor_pos = cursor_pos = input.size();
	setCursor();
}

void InputField::setCursor() {
	if (last_cursor_pos > input.size())
		last_cursor_pos = input.size();
	if (cursor_pos > input.size())
		cursor_pos = input.size();

	if (cursor_input.size() != (input.size() + 1))
		cursor_input.resize(input.size() + 1, ' ');
	cursor_input[last_cursor_pos] = ' ';
	cursor_input[cursor_pos] = input_cursor;
}

void InputField::removeCursor() {
	cursor_pos = 0;
	last_cursor_pos = 0;
	cursor_input.clear();
}

void InputField::cursorLeft()
{
	if (cursor_pos > 0) {
		last_cursor_pos = cursor_pos;
		cursor_pos--;
		setCursor();
		RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
	}
}

void InputField::cursorRight()
{
	if (cursor_pos < input.size()) {
		last_cursor_pos = cursor_pos;
		cursor_pos++;
		setCursor();
		RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
	}
}

bool InputField::popInput() {
	bool ins = cursor_pos < input.size() ? true : false;
	if (ins && (cursor_pos - 1) < 0)
		return false;
	if (InputField::p_protected) {
		ins ? InputField::pp_input.erase(InputField::pp_input.begin() + (cursor_pos - 1)) : InputField::pp_input.pop_back();
	}
	ins ? InputField::input.erase(InputField::input.begin() + (cursor_pos - 1)) : InputField::input.pop_back();
	InputField::cursor_input.pop_back();
	last_cursor_pos = cursor_pos;
	cursor_pos--;
	return true;
}

void InputField::clearInput() {
	if (InputField::p_protected) {
		InputField::pp_input.clear();
	}
	InputField::input.clear();
	InputField::cursor_input.clear();
	last_cursor_pos = 0;
	cursor_pos = 0;
}

void InputField::setInput(std::string text, bool cursor) {
	InputField::clearInput();
	if (InputField::p_protected)
	{
		for (int i = 0; i < text.size(); i++)
		{
			InputField::pp_input += '*';
		}
	}
	InputField::input = text;

	if (cursor)
	{
		last_cursor_pos = cursor_pos;
		for (int i = 0; i < text.size(); i++) {
			InputField::cursor_input += ' ';
			cursor_pos++;
		}
	}
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
		InputField::setCursor();
		RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
	}
}

bool InputField::can_type()
{
	if (line_ptr)
	{
		ChatLine* c = line_ptr;
		if (c->parent && c->parent->type == CHILD_TYPE::DISPLAY_BOX)
		{
			DisplayBox* displayBox = (DisplayBox*)c->parent;
			bool can_type = false;
			if (c->split)
			{
				auto it = std::find(displayBox->chat_lines.begin(), displayBox->chat_lines.end(), c);
				while (it != displayBox->chat_lines.end() && (*it)->split && *(it + 1) == (*it)->split)
				{
					ChatLine* split = *(it + 1);
					if (split && !split->split)
					{
						if (split->can_type())
						{
							printf("hello\n");
							return true;
						}
					}
					it++;
				}
				return false;
			}
		}
	}
	BasicInterface& param = *InputField::border;
	double aW = param.getActualWidth();
	SelectObject(hDC, *InputField::font);
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

void InputField::handleBox()
{
	ChatLine* c = line_ptr;
	c->setText(input);
	clearInput();
	setCursor();
	line_ptr = nullptr;
	if (focusChild == this)
		removeFocus();
	frame->children[index] = nullptr;
	RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
	renderDrawings = true;
	delete this;
}

void InputField::handleBox2()
{
	line_ptr = nullptr;
	if (focusChild == this)
		removeFocus();
	frame->children[index] = nullptr;
	RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
	renderDrawings = true;
	delete this;
}

void InputField::updateLine()
{
	ChatLine* c = line_ptr;
	if (c)
	{
		c->setText(input);
		RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
		renderDrawings = true;
	}
}

void InputField::updateInput(ChatLine* c)
{
	if (c)
	{
		line_ptr = c;
		input = c->getText();
		setCursor();
		RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
		renderDrawings = true;
	}
}

void InputField::handleEntry()
{
	switch (this->index)
	{
	case FP_ARRIVE_INPUT:
	case FP_DEPART_INPUT:
	{
		if (opened_fp)
		{
			PullFPData((Aircraft*)opened_fp);
		}
		break;
	}
	}
	if (focusChild == this)
	{
		removeFocus();
	}
	renderDrawings = true;
}


CloseButton::CloseButton(InterfaceFrame* frame, double width, double height) {
	CloseButton::frame = frame;
	CloseButton::focus = false;
	CloseButton::type = CHILD_TYPE::CLOSE_BUTTON;
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
		if (focusChild) {
			focusChild->removeFocus();
			RenderChild(lastFocus);
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
		focusChild = nullptr;
		renderFocus = true;
	}
}

void CloseButton::doAction() {
	CloseButton::frame->doClose();
}

void CloseButton::focusDrawing() {
}

int CloseButton::handleClick(ChildFrame* clicked, int x, int y)
{
	return 0;
}

ClickButton::ClickButton(InterfaceFrame* frame, std::string text, double x, double width, double y, double height) {
	ClickButton::frame = frame;
	ClickButton::focus = false;
	ClickButton::type = CHILD_TYPE::CLICK_BUTTON;
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
	SIZE extent = getTextExtent(this->text);
	TEXTMETRIC tm;
	GetTextMetrics(hDC, &tm);
	int tH = tm.tmAscent - tm.tmInternalLeading;
	double textXPos = x - (extent.cx / 2.0);
	double textYPos = y - ((extent.cy / 2.0) / 2.0);
	glColor4f(button_text_clr[0], button_text_clr[1], button_text_clr[2], 1.0f);
	glRasterPos2f(textXPos, textYPos);
	glPrint(ClickButton::text.c_str(), &topButtonBase);

	int start_x = param.getStartX(), end_x = param.getEndX();
	int start_y = param.getStartY(), end_y = param.getEndY();

	glBegin(GL_LINE_LOOP);
	glVertex2d(start_x + border_pix, start_y + border_pix);
	glVertex2d(start_x + border_pix, end_y - border_pix);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex2d(start_x + border_pix, end_y - border_pix);
	glVertex2d(end_x - border_pix, end_y - border_pix);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex2d(end_x - border_pix, param.getStartY() + border_pix);
	glVertex2d(end_x - border_pix, param.getEndY() - border_pix);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex2d(start_x + border_pix, start_y + border_pix);
	glVertex2d(end_x - border_pix, start_y + border_pix);
	glEnd();

	DrawVarLine(start_x, start_y, start_x + border_pix, start_y + border_pix, 1, 1);
	DrawVarLine(start_x, end_y, start_x + border_pix, end_y - border_pix, 1, 1);
	DrawVarLine(end_x, start_y, end_x - border_pix, start_y + border_pix, 1, 1);
	DrawVarLine(end_x, end_y, end_x - border_pix, end_y - border_pix, 1, 1);
}

void ClickButton::setFocus() {
	if (!ClickButton::focus) {
		if (focusChild) {
			focusChild->removeFocus();
			RenderChild(lastFocus);
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
		focusChild = nullptr;
		renderFocus = true;
	}
}
void ClickButton::doAction() {
	switch (frame->id)
	{
	case CONNECT_INTERFACE://connect frame
	{
		switch (ClickButton::index)
		{
		case CONN_OKAY_BUTTON:
		{
			Identity& id = *USER->getIdentity();
			if (connect_callsign->input.size() < 1 || connect_fullname->input.size() < 1
				|| connect_username->input.size() < 1 || connect_password->input.size() < 1)
			{
				sendErrorMessage("Missing Connection Data.");
				break;
			}
			id.callsign = connect_callsign->input.c_str();
			id.login_name = connect_fullname->input.c_str();
			id.username = connect_username->input.c_str();
			id.password = connect_password->input.c_str();
			id.controller_rating = connect_rating->pos;
			id.controller_position = connect_position->pos;
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
	case FP_INTERFACE://connect frame
	{
		switch (ClickButton::index)
		{
		case FP_ASSIGN_SQUAWK:
		{
			squawk_input->setInput(std::to_string(squawk_range++), false);
			renderAllInputText = true;
			if (opened_fp)
			{
				Aircraft* aircraft = ((Aircraft*)opened_fp);
				PullFPData(aircraft);
				aircraft->setUpdateFlag(ACF_CALLSIGN, true);

			}
		}
		break;
		case FP_AMMEND_PLAN:
		{
			if (opened_fp)
				PullFPData((Aircraft*)opened_fp);
		}
		break;
		}
	}
	break;
	}
}


void ClickButton::focusDrawing() {
}

int ClickButton::handleClick(ChildFrame* clicked, int x, int y)
{
	return 0;
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
	ComboBox::type = CHILD_TYPE::COMBO_BOX;
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
	//TEXTMETRIC tm;
	//GetTextMetrics(hDC, &tm);
	//int tH = tm.tmAscent - tm.tmInternalLeading;
	double textXPos = x - (ComboBox::extents[ComboBox::pos].cx / 2.0);
	double textYPos = y - ((ComboBox::extents[ComboBox::pos].cy / 2.0) / 2.0);
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
		if (focusChild)
		{
			focusChild->removeFocus();
			RenderChild(lastFocus);
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
		focusChild = nullptr;
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

int ComboBox::handleClick(ChildFrame* clicked, int x, int y)
{
	return 0;
}

DisplayBox::DisplayBox(InterfaceFrame* frame, double x, double width, double x_padding, double y, double height, double y_padding, bool centerText) {
	DisplayBox::frame = frame;
	DisplayBox::centered = centerText;
	DisplayBox::focus = false;
	DisplayBox::editable = false;
	DisplayBox::type = CHILD_TYPE::DISPLAY_BOX;
	BasicInterface* comboBounds = new BasicInterface(x, width, x_padding, y, height, y_padding, 1.0f, 1.0f, 1.0f, 0.8, true, true);
	comboBounds->setBounds(true);
	comboBounds->updateCoordinates();
	DisplayBox::border = comboBounds;
	DisplayBox::child_interfaces.push_back(comboBounds);
	DisplayBox::font = &topBtnFont;
	DisplayBox::base = &topButtonBase;
}

DisplayBox::~DisplayBox()
{
	auto it = chat_lines.begin();
	while (it != chat_lines.end())
	{
		ChatLine* _r = *it;
		it = chat_lines.erase(it);
		delete _r;
	}
}

void DisplayBox::prepare()
{
	BasicInterface& param = *DisplayBox::border;

	double aW = param.getActualWidth();
	SelectObject(hDC, *font);
	TEXTMETRIC tm;
	GetTextMetrics(hDC, &tm);

	long ave = tm.tmAveCharWidth;
	int maxChars = aW / ave;

	bool reset_idx = false;

	//split line
	auto it = DisplayBox::chat_lines.end();
	while (it != DisplayBox::chat_lines.begin())
	{
		--it;
		ChatLine* m = *it;
		m->reset_p();
		std::string text = m->getText();
		CHAT_TYPE type = m->getType();
		SIZE size = getTextExtent(text);
		if (size.cx >= (param.getActualWidth() - 1)) {
			std::vector<std::string> store;
			if (!wordWrap(store, text.c_str(), maxChars, 0)) {
				//std::cout << "hello" << std::endl;
				continue;
			}

			std::string new_text;
			int remaining = 0;
			int s_size = store.size();

			for (int iy = 0; iy < s_size - 1; iy++) {
				new_text += store[iy];
				remaining++;
			}

			if (remaining > 0)
			{
				m->setText(rtrim(ltrim(store[s_size - 1])));

				ChatLine* c = new ChatLine(rtrim(ltrim(new_text)), type, this);
				it = DisplayBox::chat_lines.insert(it, c) + 1;

				c->split = m;
			}
		}
	}

	//unsplit lines
	auto i = DisplayBox::chat_lines.begin();
	while (i != DisplayBox::chat_lines.end())
	{
		ChatLine* c = *i;

		bool set_pos = false;
		if (c->split) {
			ChatLine* n = *(i + 1);
			std::string next_text = n->getText();
			std::vector<std::string> store = split(next_text, " ", 1);
			if (store.size() < 1) {
				++i;
				continue;
			}
			SIZE size = getTextExtent(c->getText() + " " + store[0]);
			if (size.cx < (param.getActualWidth() - 1)) {
				c->setText(c->getText() + " " + store[0]);

				size_t pos = next_text.find(store[0]);
				if (pos != std::string::npos)
					next_text.erase(pos, store[0].length());

				//next_text = rtrim(ltrim(next_text));
				if (next_text.size() > 0) {
					n->setText(next_text);
					continue;
				}
				c->split = n->split;
				DisplayBox::chat_lines.erase(i + 1);
				delete n;
				reset_idx = true;
			}
			else ++i;
		}
		else ++i;
	}

	//prune
	auto i2 = chat_lines.begin();
	while (i2 != chat_lines.end())
	{
		if (chat_lines.size() <= numBlocks)
			break;
		ChatLine* c = *i2;
		if (empty(c->getText()))
		{
			i2 = chat_lines.erase(i2);
			delete c;
			reset_idx = true;
		}
		else ++i2;
	}

	while (chat_lines.size() < numBlocks) 
	{
		if (prune_top)
			chat_lines.insert(chat_lines.begin(), new ChatLine("", CHAT_TYPE::MAIN, this));
		else
			chat_lines.push_back(new ChatLine("", CHAT_TYPE::MAIN, this));
	}

	if (reset_idx) {
		resetReaderIdx();
		reset_idx = false;
	}
}

InputField* DisplayBox::editText(ChatLine* line, int x, int y)
{
	InputField* input_field = nullptr;
	std::string str = line->getText();

	if (frame->children[(index + 1)])
	{
		((InputField*)frame->children[(index + 1)])->handleBox();
	}

	if (empty(str))
	{
		input_field = new InputField(frame, line->get_x(), line->size_x(), 0.0, line->get_y(), line->size_y(), 0.0);
		input_field->line_ptr = line;
		input_field->show_border = false;
		input_field->offset_x = 0;
		input_field->offset_y = 0;
		input_field->caps = DisplayBox::caps;
	}
	else if ((x != -1 && y != -1) && line->in_bounds_text(x, y))
	{
		//TODO set cursor exactly 
		input_field = new InputField(frame, line->get_x(), line->size_x(), 0.0, line->get_y(), line->size_y(), 0.0);
		input_field->line_ptr = line;
		input_field->show_border = false;
		input_field->setInput(str, true);
		input_field->offset_x = 0;
		input_field->offset_y = 0;
		input_field->caps = DisplayBox::caps;
	}
	else
	{
		input_field = new InputField(frame, line->get_x(), line->size_x(), 0.0, line->get_y(), line->size_y(), 0.0);
		input_field->line_ptr = line;
		input_field->show_border = false;
		input_field->setInput(str, true);
		input_field->offset_x = 0;
		input_field->offset_y = 0;
		input_field->caps = DisplayBox::caps;
	}

	return input_field;
}

bool DisplayBox::placeEdit(InputField* input_field)
{
	if (input_field)
	{
		frame->children[input_field->index = (index + 1)] = input_field;
		input_field->setFocus();
		RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
		renderInterfaces = true;
		renderDrawings = true;
		return true;
	}
	return false;
}

void DisplayBox::setList(std::vector<ChatLine*> chat_lines, int numBlocks)
{
	DisplayBox::chat_lines = chat_lines;
	DisplayBox::numBlocks = numBlocks;
}

void DisplayBox::gen_points()
{
	BasicInterface& param = *DisplayBox::border;
	double x;
	double aW = param.getActualWidth();
	if (DisplayBox::centered) {
		x = (param.getStartX() + (aW / 2));
	}
	else {
		x = param.getStartX();
	}
	int dbox_padding = 0;
	double y_height = ((param.getActualHeight() - dbox_padding) / DisplayBox::numBlocks);

	double last_end_y = -1;


	for (size_t i = read_index; i < read_index + numBlocks; i++)
	{
		ChatLine* line = DisplayBox::chat_lines[i];
		std::string text = line->getText();
		//std::cout << text << ", " << i << std::endl;
		double y, endY;
		if (last_end_y != -1)
		{
			y = (last_end_y - (y_height / 2));
			endY = (last_end_y - y_height);
		}
		else {
			y = (param.getEndY() - (y_height / 2));
			endY = (param.getEndY() - y_height);
		}
		SelectObject(hDC, *font);
		TEXTMETRIC tm;
		GetTextMetrics(hDC, &tm);
		long ave = tm.tmAveCharWidth;
		SIZE size = getTextExtent(text);
		long long cx, cy;
		if (text.size() == 0 || (size.cx == 0 && size.cy == 0))
			cx = ave, cy = tm.tmHeight;
		else
			cx = size.cx, cy = size.cy;
		//int tH = tm.tmAscent - tm.tmInternalLeading;
		double textXPos;
		DisplayBox::centered ? (textXPos = x - (cx / 2.0)) : (textXPos = x + noncp);
		double textYPos = y - ((cy / 2.0) / 2.0);
		last_end_y = endY;

		line->set_p(textXPos, textYPos, cx, cy, param.getActualWidth(), y_height);
	}
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
	int dbox_padding = 0;
	double y_height = ((param.getActualHeight() - dbox_padding) / DisplayBox::numBlocks);
	double last_end_y = -1;
	//SelectObject(hDC, topBtnFont);
	//TEXTMETRIC tm;
	//GetTextMetrics(hDC, &tm);
	//long ave = tm.tmAveCharWidth;
	//int maxChars = aW / ave;

	prepare();


	//draw Text to screen
	displayed_lines.clear();
	for (size_t i = read_index; i < read_index + numBlocks; i++)
	{
		ChatLine* line = DisplayBox::chat_lines[i];
		std::string text = line->getText();
		CHAT_TYPE type = line->getType();
		//std::cout << text << ", " << i << std::endl;
		double y, endY;
		if (last_end_y != -1)
		{
			y = (last_end_y - (y_height / 2));
			endY = (last_end_y - y_height);
		}
		else {
			y = (param.getEndY() - (y_height / 2));
			endY = (param.getEndY() - y_height);
		}
		SelectObject(hDC, *font);
		TEXTMETRIC tm;
		GetTextMetrics(hDC, &tm);
		long ave = tm.tmAveCharWidth;
		SIZE size = getTextExtent(text);
		long long cx, cy;
		if (text.size() == 0 || (size.cx == 0 && size.cy == 0))
			cx = ave, cy = tm.tmHeight;
		else
			cx = size.cx, cy = size.cy;
		//int tH = tm.tmAscent - tm.tmInternalLeading;
		double textXPos;
		DisplayBox::centered ? (textXPos = x - (cx / 2.0)) : (textXPos = x + noncp);
		double textYPos = y - ((cy / 2.0) / 2.0);
		DisplayBox::SetChatTextColour(type);
		glRasterPos2f(textXPos, textYPos);
		glPrint(text.c_str(), base);
		last_end_y = endY;

		line->set_p(textXPos, textYPos, cx, cy, param.getActualWidth(), y_height);

		displayed_lines.push_back(line);
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
		if (focusChild) {
			focusChild->removeFocus();
			RenderChild(lastFocus);
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
		focusChild = nullptr;
		renderFocus = true;
	}
}

void DisplayBox::doAction() {
}

void DisplayBox::focusDrawing() {
	if (DisplayBox::focus) {
		int offsetX = 4, offsetY = 4;
		BasicInterface& param = *DisplayBox::border;
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

int DisplayBox::handleClick(ChildFrame* clicked, int x, int y)
{
	int arrow_bounds = 15, arrow_offset = 3;
	if (click_arrow_bottom(x, y, arrow_bounds, arrow_offset)) {
		clicked = this;
		doActionDown();
		return 1;
	}

	if (click_arrow_top(x, y, arrow_bounds, arrow_offset)) {
		clicked = this;
		doActionUp();
		return 1;
	}
	for (auto d_it = displayed_lines.begin(); d_it != displayed_lines.end(); ++d_it) {
		ChatLine* line = *d_it;
		std::string str = line->getText();

		if (line->in_bounds(x, y))
		{
			if (editable)
			{

				if (frame->children[index + 1])
				{
					((InputField*)frame->children[index + 1])->handleBox();
				}
				InputField* input_field = editText(line, x, y);
				placeEdit(input_field);
			}
			else
			{
				switch (this->frame->id)
				{
				case CONTROLLER_INTERFACE:
					switch (this->index)
					{
					case CONTROLLER_LIST_BOX:
						//handle controller click
						controller_info_box->clearLines();

						if (!str.empty())
						{

							std::string callsign = trim(str.substr(6, 16));

							if (!callsign.empty())
							{

								Controller* controller_selected = controller_map[callsign];

								if (controller_selected)
								{
									controller_info_box->addLineTop(new ChatLine(
										"Vis Range: " + std::to_string(controller_selected->getVisibility()),
										CHAT_TYPE::MAIN, controller_info_box));
									controller_info_box->addLineTop(new ChatLine(" ", CHAT_TYPE::MAIN, controller_info_box));
									controller_info_box->addLineTop(new ChatLine(" ", CHAT_TYPE::MAIN, controller_info_box));
									controller_info_box->addLineTop(new ChatLine(controller_selected->getIdentity()->login_name
										+ " (" + CONTROLLER_RATINGS[controller_selected->getIdentity()->controller_rating] + ")",
										CHAT_TYPE::MAIN, controller_info_box));

									controller_info_box->addLineTop(new ChatLine(controller_selected->getCallsign(),
										CHAT_TYPE::MAIN, controller_info_box));
								}
							}
						}
						else
						{

						}

						renderDrawings = true;
						break;
					}
					break;
				}
			}
			clicked = this;
			return 1;
		}
	}
	return 0;// returning 1 means break any outside loop
}

void DisplayBox::addLine(ChatLine* c) {
	if (DisplayBox::chat_lines.size() >= max_history)
		DisplayBox::chat_lines.erase(DisplayBox::chat_lines.begin());
	DisplayBox::chat_lines.push_back(c);
	if ((read_index + numBlocks) < max_history)
		read_index++;
}

void DisplayBox::addLineTop(ChatLine* c) {
	if (DisplayBox::chat_lines.size() >= max_history)
		DisplayBox::chat_lines.pop_back();
	DisplayBox::chat_lines.insert(DisplayBox::chat_lines.begin(), c);
	if ((read_index + numBlocks) < max_history)
		read_index++;
}

void DisplayBox::removeLine(ChatLine* c)
{
	//std::cout << DisplayBox::chat_lines.size() << ", " << numBlocks << std::endl;
	if (DisplayBox::chat_lines.size() <= numBlocks)
	{
		c->setText("");
		c->setType(CHAT_TYPE::MAIN);
	}
	else
	{
		auto it = std::find(std::begin(DisplayBox::chat_lines), std::end(DisplayBox::chat_lines), c);

		if (it != std::end(DisplayBox::chat_lines))
		{
			it = DisplayBox::chat_lines.erase(it);
			delete c;
		}
	}
}

void DisplayBox::clearLines()
{
	resetReaderIdxTop();
	for (size_t i = read_index; i < read_index + numBlocks; i++)
	{
		ChatLine* line = DisplayBox::chat_lines[i];
		line->setText("");
	}
	auto p_it = DisplayBox::chat_lines.begin() + (read_index + numBlocks);
	while (p_it != DisplayBox::chat_lines.end())
	{
		ChatLine* line = *p_it;
		p_it = DisplayBox::chat_lines.erase(p_it);
		delete line;
	}
}

void DisplayBox::updatePos(double x, double width, double y, double height)
{
	DisplayBox::border->setPosX(x), DisplayBox::border->setPosY(y);
	DisplayBox::border->setWidth(width), DisplayBox::border->setHeight(height);
	DisplayBox::border->updateCoordinates();
}




void DisplayBox::addLine(std::string text, CHAT_TYPE type) {
	ChatLine* c = new ChatLine(text, type, this);
	addLine(c);
}

void DisplayBox::addLineTop(std::string text, CHAT_TYPE type) {
	ChatLine* c = new ChatLine(text, type, this);
	addLineTop(c);
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

void DisplayBox::resetReaderIdxTop()
{
	read_index = 0;
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
	case CHAT_TYPE::SUP_POS:
	{
		glColor4f(text_suppos_clr[0], text_suppos_clr[1], text_suppos_clr[2], 1.0f);
	}
	break;
	default:
	{
		glColor4f(button_text_clr[0], button_text_clr[1], button_text_clr[2], 1.0f);
	}
	break;
	}
}

bool DisplayBox::click_arrow_bottom(int x, int y, int arrow_bounds, int arrow_offset) {
	bool clicked = false;
	BasicInterface& inter2 = *this->border;
	double vertxt[4] = {
		inter2.getStartX() + inter2.getActualWidth() + arrow_offset,
		inter2.getStartX() + inter2.getActualWidth() + arrow_offset,
		inter2.getStartX() + inter2.getActualWidth() + (arrow_bounds)+arrow_offset,
		inter2.getStartX() + inter2.getActualWidth() + (arrow_bounds)+arrow_offset
	};
	double vertyt[4] = {
		inter2.getStartY(),
		inter2.getStartY() + arrow_bounds,
		inter2.getStartY() + arrow_bounds,
		inter2.getStartY()
	};
	bool clicked_bottom = pnpoly(4, vertxt, vertyt, x, y);
	if (clicked_bottom) {
		clicked = true;
	}
	return clicked;
}

bool DisplayBox::click_arrow_top(int x, int y, int arrow_bounds, int arrow_offset) {
	bool clicked = false;
	BasicInterface& inter2 = *this->border;
	double vertx[4] = {
		inter2.getStartX() + inter2.getActualWidth() + arrow_offset,
		inter2.getStartX() + inter2.getActualWidth() + arrow_offset,
		inter2.getStartX() + inter2.getActualWidth() + (arrow_bounds)+arrow_offset,
		inter2.getStartX() + inter2.getActualWidth() + (arrow_bounds)+arrow_offset
	};
	double verty[4] = {
		inter2.getStartY() + inter2.getActualHeight(),
		inter2.getStartY() + inter2.getActualHeight() - arrow_bounds,
		inter2.getStartY() + inter2.getActualHeight() - arrow_bounds,
		inter2.getStartY() + inter2.getActualHeight()
	};
	bool clicked_top = pnpoly(4, vertx, verty, x, y);
	if (clicked_top) {
		clicked = true;
	}
	return clicked;
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
	Label::type = CHILD_TYPE::LABEL_D;
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

int Label::handleClick(ChildFrame* clicked, int x, int y)
{
	return 0;
}

ChatLine::ChatLine(std::string line, CHAT_TYPE type, ChildFrame* parent)
{
	ChatLine::line = line;
	ChatLine::type = type;
	ChatLine::parent = parent;
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
		PlaySound(MAKEINTRESOURCE(IDW_SOUND3), NULL, SND_RESOURCE | SND_ASYNC);
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
	case CHAT_TYPE::CHAT:
	{
		PlaySound(MAKEINTRESOURCE(IDW_SOUND4), NULL, SND_RESOURCE | SND_ASYNC);
	}
	break;
	default:
	{
	}
	break;
	}
}

bool ChatLine::in_bounds(int x, int y)
{
	double vertx[4] = { _x, _x, (_x + _p_x), (_x + _p_x) };
	double verty[4] = { _y, (_y + _p_y), (_y + _p_y), _y };
	return pnpoly(4, vertx, verty, x, y);
}

bool ChatLine::in_bounds_text(int x, int y)
{
	double vertx[4] = { _x, _x, (_x + _s_x), (_x + _s_x) };
	double verty[4] = { _y, (_y + _s_y), (_y + _s_y), _y };
	return pnpoly(4, vertx, verty, x, y);
}

bool ChatLine::can_type()
{
	if (parent)
	{
		BasicInterface& param = *ChatLine::parent->border;
		double aW = param.getActualWidth();
		SelectObject(hDC, *parent->font);
		TEXTMETRIC tm;
		GetTextMetrics(hDC, &tm);
		long ave = tm.tmAveCharWidth;
		int maxChars = aW / ave;
		if (ChatLine::line.size() < (maxChars - 1))
			return true;
	}
	return false;
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

