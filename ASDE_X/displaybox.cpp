#include "displaybox.h"

#include <utility>

#include "controller.h"
#include "tools.h"

DisplayBox::DisplayBox(InterfaceFrame* frame, double x, double width, double x_padding, double y, double height, double y_padding, bool centerText) {
	DisplayBox::frame = frame;
	centered = centerText;
	focus = false;
	editable = false;
	type = CHILD_TYPE::DISPLAY_BOX;
	auto* comboBounds = new BasicInterface(x, width, x_padding, y, height, y_padding, 1.0f, 1.0f, 1.0f, 0.8, true, true);
	comboBounds->setBounds(true);
	comboBounds->updateCoordinates();
	border = comboBounds;
	child_interfaces.push_back(comboBounds);
	font = &topBtnFont;
	base = &topButtonBase;
}

DisplayBox::~DisplayBox()
{
	auto it = chat_lines.begin();
	while (it != chat_lines.end()) {
		std::shared_ptr<ChatLine>& _r = *it;
		it = chat_lines.erase(it);
	}
}

std::shared_ptr<ChatLine> DisplayBox::check_unsplit()
{
	BasicInterface& param = *border;

	double aW = param.getActualWidth();
	SelectObject(hDC, *font);
	TEXTMETRIC tm;
	GetTextMetrics(hDC, &tm);

	long ave = tm.tmAveCharWidth;
	int maxChars = aW / ave;

	bool reset_idx = false;

	std::shared_ptr<ChatLine> u = nullptr;

	//unsplit lines
	auto it = chat_lines.end();
	while (it != chat_lines.begin()) {
		--it;
		std::shared_ptr<ChatLine>& m = *it;
		m->reset_p();
		std::string text = m->getText();
		CHAT_TYPE type = m->getType();
		SIZE size = getTextExtent(text);
		if (size.cx >= (aW - 1)) {
			std::vector<std::string> store;
			store.reserve(maxChars);
			if (!wordWrap(store, text.c_str(), maxChars, 0)) {
				continue;
			}

			// Handle the split lines and insert new lines if necessary
			if (!store.empty()) {
				m->setText(store[0]);
				for (size_t i = 1; i < store.size(); ++i) {
					auto c = std::make_shared<ChatLine>(store[i], type, this);
					c->split = m;
					it = chat_lines.insert(it + 1, c);
				}
			}
		}
	}

	prune(reset_idx);

	if (reset_idx) {
		resetReaderIdx();
		reset_idx = false;
	}

	return u;
}

void DisplayBox::consolidate_lines() {
	BasicInterface& param = *border;

	double aW = param.getActualWidth();
	SelectObject(hDC, *font);
	TEXTMETRIC tm;
	GetTextMetrics(hDC, &tm);

	long ave = tm.tmAveCharWidth;
	int maxChars = aW / ave;

	bool reset_idx = false;

	//split line
	auto it = chat_lines.end();
	while (it != chat_lines.begin()) {
		--it;
		std::shared_ptr<ChatLine>& m = *it;
		m->reset_p();
		std::string text = m->getText();
		CHAT_TYPE type = m->getType();
		SIZE size = getTextExtent(text);
		if (size.cx >= (aW - 1)) {
			std::vector<std::string> store;
			store.reserve(maxChars);
			if (!wordWrap(store, text.c_str(), maxChars, 0)) {
				//std::cout << "hello" << std::endl;
				continue;
			}

			std::stringstream new_text_ss;
			int remaining = 0;
			int s_size = store.size();

			for (int iy = 0; iy < s_size - 1; iy++) {
				new_text_ss << store[iy];
				remaining++;
			}

			std::string new_text = new_text_ss.str();

			if (remaining > 0)
			{
				m->setText(rtrim(ltrim(store[s_size - 1])));

				auto c = std::make_shared<ChatLine>(rtrim(ltrim(new_text)), type, this);
				c->split = m;
				it = chat_lines.insert(it, c) + 1;
			}
		}
	}

	//unsplit lines
	auto i = chat_lines.begin();
	while (i != chat_lines.end()) {
		std::shared_ptr<ChatLine>& c = *i;

		bool set_pos = false;
		if (c->split) {
			std::shared_ptr<ChatLine>& n = *(i + 1);
			std::string next_text = n->getText();
			std::vector<std::string> store = split(next_text, " ", 1);
			if (store.empty()) {
				++i;
				continue;
			}

			std::stringstream size_ss;
			size_ss << c->getText() << " " << store[0];
			SIZE size = getTextExtent(size_ss.str());
			if (size.cx < (aW - 1)) {
				std::stringstream text_ss;
				text_ss << c->getText() << " " << store[0];
				c->setText(text_ss.str());

				size_t pos = next_text.find(store[0]);
				if (pos != std::string::npos)
					next_text.erase(pos, store[0].length());

				next_text = rtrim(ltrim(next_text));
				if (!next_text.empty()) {
					n->setText(next_text);
					continue;
				}
				c->split = n->split;
				chat_lines.erase(i + 1);
				reset_idx = true;
			}
			else ++i;
		}
		else ++i;
	}

	//prune
	prune(reset_idx);

	if (reset_idx) {
		resetReaderIdx();
		reset_idx = false;
	}
}

void DisplayBox::prune(bool &reset_idx)
{
	//prune
	size_t chat_lines_size = chat_lines.size();
	auto i2 = chat_lines.begin();
	while (i2 != chat_lines.end()) {
		if (chat_lines_size <= numBlocks)
			break;
		std::shared_ptr<ChatLine>& c = *i2;
		if (empty(c->getText()))
		{
			i2 = chat_lines.erase(i2);
			--chat_lines_size;
			reset_idx = true;
		}
		else ++i2;
	}

	while (chat_lines_size < numBlocks) {
		if (prune_top)
			chat_lines.insert(chat_lines.begin(), std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, this));
		else
			chat_lines.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, this));

		++chat_lines_size;
	}
}

InputField* DisplayBox::edit_text(std::shared_ptr<ChatLine> line, int x, int y) const
{
	InputField* input_field = nullptr;
	const std::string str = line->getText();

	if (frame->children[(index + 1)])
	{
		dynamic_cast<InputField*>(frame->children[(index + 1)])->handle_box();
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
		SelectObject(hDC, *font);
		const SIZE extent = getTextExtent(line->getText());
		const double x_pos = ((static_cast<double>(x) - line->get_x()) / extent.cx) * line->getText().size();
		input_field = new InputField(frame, line->get_x(), line->size_x(), 0.0, line->get_y(), line->size_y(), 0.0);
		input_field->line_ptr = line;
		input_field->show_border = false;
		input_field->setInput(str);
		input_field->offset_x = 0;
		input_field->offset_y = 0;
		input_field->caps = caps;
		input_field->cursor_default = false;
		input_field->setCursor(x_pos);
	}
	else
	{
		input_field = new InputField(frame, line->get_x(), line->size_x(), 0.0, line->get_y(), line->size_y(), 0.0);
		input_field->line_ptr = line;
		input_field->show_border = false;
		input_field->setInput(str);
		input_field->offset_x = 0;
		input_field->offset_y = 0;
		input_field->caps = caps;
	}

	return input_field;
}

bool DisplayBox::placeEdit(InputField* input_field) const
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

std::string DisplayBox::getLinesComb()
{
	std::string all_lines;
	for (auto it = chat_lines.begin(); it != chat_lines.end(); ++it) {
		it != chat_lines.begin() ? all_lines += " " + (*it)->getText() : all_lines += (*it)->getText();
	}
	return all_lines;
}

void DisplayBox::setList(std::vector<std::shared_ptr<ChatLine>> chat_lines, int numBlocks)
{
	DisplayBox::chat_lines = chat_lines;
	DisplayBox::numBlocks = numBlocks;
}

void DisplayBox::gen_points() const
{
	BasicInterface& param = *border;
	double x;
	double aW = param.getActualWidth();
	if (centered) {
		x = (param.getStartX() + (aW / 2));
	}
	else {
		x = param.getStartX();
	}
	int dbox_padding = 0;
	double y_height = ((param.getActualHeight() - dbox_padding) / numBlocks);

	double last_end_y = -1;


	for (size_t i = read_index; i < read_index + numBlocks; i++)
	{
		const std::shared_ptr<ChatLine>& line = chat_lines[i];
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
		if (text.empty() || (size.cx == 0 && size.cy == 0))
			cx = ave, cy = tm.tmHeight;
		else
			cx = size.cx, cy = size.cy;
		//int tH = tm.tmAscent - tm.tmInternalLeading;
		double textXPos;
		centered ? (textXPos = x - (cx / 2.0)) : (textXPos = x + noncp);
		const double textYPos = y - ((cy / 2.0) / 2.0);
		last_end_y = endY;

		line->set_p(textXPos, textYPos, cx, cy, param.getActualWidth(), y_height);
	}
}

void DisplayBox::doDrawing() {
	BasicInterface& param = *border;
	double x;
	double aW = param.getActualWidth();
	if (centered) {
		x = (param.getStartX() + (aW / 2));
	}
	else {
		x = param.getStartX();
	}
	const int dbox_padding = 0;
	const double y_height = ((param.getActualHeight() - dbox_padding) / numBlocks);
	double last_end_y = -1;
	//SelectObject(hDC, topBtnFont);
	//TEXTMETRIC tm;
	//GetTextMetrics(hDC, &tm);
	//long ave = tm.tmAveCharWidth;
	//int maxChars = aW / ave;

	consolidate_lines();


	//draw Text to screen
	displayed_lines.clear();
	for (size_t i = read_index; i < read_index + numBlocks; i++)
	{
		std::shared_ptr<ChatLine>& line = chat_lines[i];
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
	if (!focus) {
		if (focusChild) {
			focusChild->removeFocus();
			RenderChild(lastFocus);
		}
		focus = true;
		focusChild = this;
		renderFocus = true;
	}
}

void DisplayBox::removeFocus() {
	if (focus) {
		//TODO Design

		focus = false;
		lastFocus = this;
		focusChild = nullptr;
		renderFocus = true;
	}
}

void DisplayBox::doAction() {
}

void DisplayBox::focusDrawing() {
	if (focus) {
		constexpr int offset_x = 4;
		constexpr int offset_y = 4;
		BasicInterface& param = *border;
		const double x1 = (param.getStartX() - offset_x);
		const double x2 = (param.getStartX() + param.getActualWidth()) + offset_x;
		const double y1 = (param.getStartY() - offset_y);
		const double y2 = (param.getStartY() + param.getActualHeight()) + offset_y;
		glColor3f(0.32549019607f, 0.03137254901f, 0.91372549019f);
		constexpr int line_size = 2;
		DrawVarLine(x1, y1, x1, y2, line_size, line_size);
		DrawVarLine(x1, y2, x2, y2, line_size, line_size);
		DrawVarLine(x2, y2, x2, y1, line_size, line_size);
		DrawVarLine(x2, y1, x1, y1, line_size, line_size);
	}
}

int DisplayBox::handleClick(ChildFrame* clicked, int x, int y)
{
	constexpr int arrow_bounds = 15;
	constexpr int arrow_offset = 3;
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
		std::shared_ptr<ChatLine>& line = *d_it;
		const std::string str = line->getText();

		if (line->in_bounds(x, y)) {
			if (editable) {
				if (frame->children[index + 1]) {
					dynamic_cast<InputField*>(frame->children[index + 1])->handle_box();
				}
				InputField* input_field = edit_text(line, x, y);
				placeEdit(input_field);

				clicked = this;
				return 1;
			}
			else {
				if (this->frame->id == CONTROLLER_INTERFACE) {
					if (this->index == CONTROLLER_LIST_BOX) {
						//handle controller click
						controller_info_box->clearLines();

						if (!str.empty()) {
							if (const std::string call_sign = trim(str.substr(6, 16)); !call_sign.empty()) {
								if (Controller* controller_selected = controller_map[call_sign]) {
									controller_info_box->addLineTop(std::make_shared<ChatLine>(
										"Vis Range: " + std::to_string(controller_selected->getVisibility()),
										CHAT_TYPE::MAIN, controller_info_box));
									controller_info_box->addLineTop(std::make_shared<ChatLine>(" ", CHAT_TYPE::MAIN, controller_info_box));
									controller_info_box->addLineTop(std::make_shared<ChatLine>(" ", CHAT_TYPE::MAIN, controller_info_box));
									controller_info_box->addLineTop(std::make_shared<ChatLine>(controller_selected->getIdentity()->login_name
										+ " (" + CONTROLLER_RATINGS[controller_selected->getIdentity()->controller_rating] + ")",
										CHAT_TYPE::MAIN, controller_info_box));

									controller_info_box->addLineTop(std::make_shared<ChatLine>(controller_selected->getCallsign(),
										CHAT_TYPE::MAIN, controller_info_box));
								}
							}
						}
						else {

						}

						renderDrawings = true;
						clicked = this;
						return 1;
					}
				}
			}
		}
	}
	return 0;// returning 1 means break any outside loop
}

void DisplayBox::addLine(const std::shared_ptr<ChatLine>& c) {
	if (chat_lines.size() >= max_history)
		chat_lines.erase(chat_lines.begin());
	chat_lines.push_back(c);
	if ((read_index + numBlocks) < max_history)
		read_index++;
}

void DisplayBox::addLineTop(const std::shared_ptr<ChatLine>& c) {
	if (chat_lines.size() >= max_history)
		chat_lines.pop_back();
	chat_lines.insert(chat_lines.begin(), c);
	if ((read_index + numBlocks) < max_history)
		read_index++;
}

void DisplayBox::removeLine(const std::shared_ptr<ChatLine>& c)
{
	//std::cout << DisplayBox::chat_lines.size() << ", " << numBlocks << std::endl;
	if (chat_lines.size() <= numBlocks)
	{
		c->setText("");
		c->setType(CHAT_TYPE::MAIN);
	}
	else
	{
		auto it = std::find(std::begin(chat_lines), std::end(chat_lines), c);

		if (it != std::end(chat_lines))
		{
			it = chat_lines.erase(it);
		}
	}
}

void DisplayBox::clearLines()
{
	resetReaderIdxTop();
	for (size_t i = read_index; i < read_index + numBlocks; i++)
	{
		std::shared_ptr<ChatLine>& line = chat_lines[i];
		line->setText("");
	}
	auto p_it = chat_lines.begin() + (read_index + numBlocks);
	while (p_it != chat_lines.end())
	{
		std::shared_ptr<ChatLine>& line = *p_it;
		p_it = chat_lines.erase(p_it);
	}
}

void DisplayBox::updatePos(double x, double width, double y, double height)
{
	border->setPosX(x), border->setPosY(y);
	border->setWidth(width), border->setHeight(height);
	border->updateCoordinates();
}


void DisplayBox::addLine(const std::string& text, CHAT_TYPE type) {
	addLine(std::make_shared<ChatLine>(text, type, this));
}

void DisplayBox::addLineTop(const std::string& text, CHAT_TYPE type) {
	addLineTop(std::make_shared<ChatLine>(text, type, this));
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
	return pnpoly(4, vertxt, vertyt, x, y);
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
	return pnpoly(4, vertx, verty, x, y);
}