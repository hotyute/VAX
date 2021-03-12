#include <vector>
#include <string>

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
public:
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
};

extern std::vector<TopButton*> BUTTONS;

extern void loadButtons();