#ifndef DISPLAYBOX_H
#define DISPLAYBOX_H

#include "gui.h"
#include "inputfield.h"

class DisplayBox : public ChildFrame {
private:
	int noncp = 2;
public:
	DisplayBox(InterfaceFrame* frame, double, double, double, double, double, double, bool);
	~DisplayBox();
public:
	int numBlocks = 0;
	int read_index = 0, max_history = 100;
	bool centered, editable = false, prune_top = false, caps = false;
	std::vector<ChatLine*> chat_lines;
	std::vector<ChatLine*> displayed_lines;

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
	void addLine(std::string, CHAT_TYPE type);
	void addLineTop(std::string, CHAT_TYPE type);
	void addLine(ChatLine* c);
	void addLineTop(ChatLine* c);
	void removeLine(ChatLine* c);
	void SetChatTextColour(CHAT_TYPE t);
	void clearLines();
	void prepare();
	InputField* editText(ChatLine* line, int x, int y);
	bool placeEdit(InputField* input_field);
	std::string getLinesComb();
	void setList(std::vector<ChatLine*> chat_lines, int numBlocks);
	void gen_points();

	bool click_arrow_bottom(int x, int y, int arrow_bounds, int arrow_offset);
	bool click_arrow_top(int x, int y, int arrow_bounds, int arrow_offset);

};

#endif
