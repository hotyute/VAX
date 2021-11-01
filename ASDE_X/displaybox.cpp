#include "displaybox.h"

#include "tools.h"

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
		SelectObject(hDC, *font);
		SIZE extent = getTextExtent(line->getText());
		double x_pos = ((((double)x) - line->get_x()) / extent.cx) * line->getText().size();
		input_field = new InputField(frame, line->get_x(), line->size_x(), 0.0, line->get_y(), line->size_y(), 0.0);
		input_field->line_ptr = line;
		input_field->show_border = false;
		input_field->setInput(str);
		input_field->offset_x = 0;
		input_field->offset_y = 0;
		input_field->caps = DisplayBox::caps;
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