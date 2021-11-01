#include "inputfield.h"

#include "tools.h"

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
		if (cursor_default)
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
	clamp_cursor();

	if (cursor_input.size() != (input.size() + 1))
		cursor_input.resize(input.size() + 1, ' ');
	cursor_input[last_cursor_pos] = ' ';
	cursor_input[cursor_pos] = input_cursor;
}

void InputField::setCursor(int pos) {
	if (pos > input.size())
		pos = input.size();
	else if (pos < 0)
		pos = 0;

	clamp_cursor();
	cursor_input[last_cursor_pos = cursor_pos] = ' ';
	cursor_pos = pos;
	clamp_cursor();

	if (cursor_input.size() != (input.size() + 1))
		cursor_input.resize(input.size() + 1, ' ');
	cursor_input[cursor_pos] = input_cursor;
}

void InputField::clamp_cursor()
{
	if (last_cursor_pos < 0)
		last_cursor_pos = 0;
	if (last_cursor_pos > input.size())
		last_cursor_pos = input.size();
	if (cursor_pos < 0)
		cursor_pos = 0;
	else if (cursor_pos > input.size())
		cursor_pos = input.size();
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

void InputField::setInput(std::string text) {
	InputField::clearInput();
	for (char& s : text)
		pushInput(false, s);
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

void InputField::calcCursorPos(double x, double y)
{
	SelectObject(hDC, *font);
	SIZE extent = getTextExtent(input);
	int _x;
	double aW = InputField::border->getActualWidth();
	if (centered) {
		_x = (InputField::border->getStartX() + (aW / 2));
	}
	else {
		_x = InputField::border->getStartX() + offset_x;
	}
	double x_pos = ((((double)x) - _x) / extent.cx) * input.size();
	setCursor(x_pos);
}
