#pragma once

#include "gui.h"
#include "inputfield.h"

class DisplayBox : public ChildFrame {
	int noncp = 2;
public:
	DisplayBox(InterfaceFrame* frame, double, double, double, double, double, double, bool);
	~DisplayBox();
	std::shared_ptr<ChatLine> check_unsplit();
	void consolidate_lines();
	void Wrap_Combined();
	void prune(bool& reset_idx);

	int numBlocks = 0;
	int read_index = 0, max_history = 100;
	bool centered, editable = false, prune_top = false, caps = false;
	bool combined_lines = false;
	std::vector<std::shared_ptr<ChatLine>> chat_lines;
	std::vector<std::shared_ptr<ChatLine>> displayed_lines;

	void updatePos(double x, double width, double y, double height);
	void doDrawing();
	void setFocus();
	void removeFocus();
	void doAction();
	void focusDrawing();
	int handleClick(ChildFrame* clicked, int x, int y);

	void resetReaderIdx();
	void resetReaderIdxTop();
	void doActionUp();
	void doActionDown();
	void addLine(const std::string&, CHAT_TYPE type);
	void addLineTop(const std::string&, CHAT_TYPE type);
	void addLine(const std::shared_ptr<ChatLine>& c);
	void addLineTop(const std::shared_ptr<ChatLine>& c);
	void removeLine(const std::shared_ptr<ChatLine>& c);
	void SetChatTextColour(CHAT_TYPE t);
	void clearLines();
	InputField* edit_text(std::shared_ptr<ChatLine> line, int x, int y) const;
	bool placeEdit(InputField* input_field) const;
	std::string getLinesComb();
	void setList(std::vector<std::shared_ptr<ChatLine>> chat_lines, int numBlocks);
	void gen_points() const;

	bool click_arrow_bottom(int x, int y, int arrow_bounds, int arrow_offset);
	bool click_arrow_top(int x, int y, int arrow_bounds, int arrow_offset);

};
