#ifndef TOPBUTTON_H
#define TOPBUTTON_H

#include <vector>
#include <string>

enum class TOP_TYPE { 
	RANGE_BTN, MODE_BTN, MAP_BTN, UNDO_BTN, DEFAULT1_BTN, PREF_BTN,
	ROTATE_BTN, DAY_NITE_BTN, BRITE_BTN, CHAR_SIZE_BTN, SAFETY_BTN,
	TOOLS_BTN, VECTOR_BTN, VECTOR2_BTN, TEMP_BTN, LDR_BTN, DB_AREA_BTN,
	DB_BTN, DCB_BTN, TERM_BTN, LOCAL_BTN, LOCAL2_BTN, OPER_BTN, DB_EDIT,
	TRKSUP, INITCNTL
};

class TopButton {
private:
	int index;
	int x;
	int y;
	double width;
	bool half;
	bool top;
	bool dark;
	bool dual_option, triple_option;
	std::string option1;
	std::string choice1;
	std::string option2;
	std::string choice2;
	std::string option3;
	int params[4];
	TOP_TYPE type;
public:
	bool off;
	TopButton(TOP_TYPE type);
	int getIndex();
	void setIndex(int);
	double getWidth();
	void setWidth(double);
	bool getIsHalf();
	void setHalf(bool);
	bool getIsTop();
	void setTop(bool);
	bool getIsDark();
	void setDark(bool);
	bool isDualOption();
	void setDualOption(bool);
	bool isTripleOption();
	void setTripleOption(bool);
	std::string getOption1();
	void setOption1(std::string);
	std::string getChoice1();
	void setChoice1(std::string);
	std::string getOption2();
	void setOption2(std::string);
	std::string getChoice2();
	void setChoice2(std::string);
	std::string getOption3();
	void setOption3(std::string);
	int* getParams() { return params; }
	void updateParams(int x, int y, int x2, int y2) { params[0] = x, params[1] = y, params[2] = x2, params[3] = y2; }
	TOP_TYPE getType() { return type; }
	void handle();
};

extern std::vector<TopButton*> BUTTONS;

extern void loadButtons();

#endif;