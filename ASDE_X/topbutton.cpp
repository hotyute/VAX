#include "topbutton.h"
#include "projection.h"
#include "tools.h"

std::vector<TopButton*> BUTTONS;

TopButton* rangeb = nullptr, * rotateb = nullptr;

std::string text;



void loadButtons() {
	rangeb = new TopButton(TOP_TYPE::RANGE_BTN);
	rangeb->setIndex(0);
	rangeb->setWidth(0.069259012016);
	rangeb->setHalf(false);
	rangeb->setTop(false);
	rangeb->setDark(true);
	rangeb->setDualOption(true);
	rangeb->setTripleOption(false);
	rangeb->setOption1("RANGE");
	if (zoom_phase < 2)
		rangeb->setOption2("NSET");
	else
		rangeb->setOption2(std::to_string(rangeb->value = range));
	BUTTONS.push_back(rangeb);
	rotateb = new TopButton(TOP_TYPE::ROTATE_BTN);
	rotateb->setIndex(1);
	rotateb->setWidth(0.069259012016);
	rotateb->setHalf(true);
	rotateb->setTop(false);
	rotateb->setDark(true);
	rotateb->setDualOption(false);
	rotateb->setTripleOption(false);
	rotateb->setOption1("ROTATE");
	rotateb->value = rotation;
	BUTTONS.push_back(rotateb);
	TopButton* map = new TopButton(TOP_TYPE::MAP_BTN);
	map->setIndex(1);
	map->setWidth(0.069259012016);
	map->setHalf(true);
	map->setTop(true);
	map->setDark(true);
	map->setDualOption(true);
	map->setTripleOption(false);
	map->setOption1("MAP");
	map->setOption2("RPOS");
	BUTTONS.push_back(map);
	TopButton* undo = new TopButton(TOP_TYPE::UNDO_BTN);
	undo->setIndex(2);
	undo->setWidth(0.069259012016);
	undo->setHalf(true);
	undo->setTop(true);
	undo->setDark(true);
	undo->setDualOption(false);
	undo->setTripleOption(false);
	undo->setOption1("UNDO");
	BUTTONS.push_back(undo);
	TopButton* default1 = new TopButton(TOP_TYPE::DEFAULT1_BTN);
	default1->setIndex(2);
	default1->setWidth(0.069259012016);
	default1->setHalf(true);
	default1->setTop(false);
	default1->setDark(true);
	default1->setDualOption(false);
	default1->setTripleOption(false);
	default1->setOption1("DEFAULT");
	BUTTONS.push_back(default1);
	TopButton* pref = new TopButton(TOP_TYPE::PREF_BTN);
	pref->setIndex(3);
	pref->setWidth(0.069259012016);
	pref->setHalf(true);
	pref->setTop(true);
	pref->setDark(false);
	pref->setDualOption(true);
	pref->setTripleOption(false);
	pref->setOption1("PREF");
	pref->setOption2("");
	BUTTONS.push_back(pref);
	TopButton* daytnite = new TopButton(TOP_TYPE::DAY_NITE_BTN);
	daytnite->setIndex(3);
	daytnite->setWidth(0.069259012016);
	daytnite->setHalf(true);
	daytnite->setTop(false);
	daytnite->setDark(true);
	daytnite->setDualOption(false);
	daytnite->setTripleOption(false);
	daytnite->setOption1("DAY");
	daytnite->setChoice1("NITE");
	daytnite->on = DAY;
	BUTTONS.push_back(daytnite);
	TopButton* brite = new TopButton(TOP_TYPE::BRITE_BTN);
	brite->setIndex(4);
	brite->setWidth(0.069259012016);
	brite->setHalf(true);
	brite->setTop(true);
	brite->setDark(false);
	brite->setDualOption(false);
	brite->setTripleOption(false);
	brite->setOption1("BRITE");
	BUTTONS.push_back(brite);
	TopButton* charsize = new TopButton(TOP_TYPE::CHAR_SIZE_BTN);
	charsize->setIndex(4);
	charsize->setWidth(0.069259012016);
	charsize->setHalf(true);
	charsize->setTop(false);
	charsize->setDark(false);
	charsize->setDualOption(true);
	charsize->setTripleOption(false);
	charsize->setOption1("CHAR");
	charsize->setOption2("SIZE");
	BUTTONS.push_back(charsize);
	TopButton* safety = new TopButton(TOP_TYPE::SAFETY_BTN);
	safety->setIndex(5);
	safety->setWidth(0.069259012016);
	safety->setHalf(false);
	safety->setTop(false);
	safety->setDark(false);
	safety->setDualOption(false);
	safety->setTripleOption(true);
	safety->setOption1("LOGIC");
	safety->setOption2("SAFETY");
	safety->setOption3("08R09-00");
	BUTTONS.push_back(safety);
	TopButton* tools = new TopButton(TOP_TYPE::TOOLS_BTN);
	tools->setIndex(6);
	tools->setWidth(0.069259012016);
	tools->setHalf(false);
	tools->setTop(false);
	tools->setDark(false);
	tools->setDualOption(false);
	tools->setTripleOption(false);
	tools->setOption1("TOOLS");
	BUTTONS.push_back(tools);
	TopButton* vector = new TopButton(TOP_TYPE::VECTOR_BTN);
	vector->setIndex(7);
	vector->setWidth(0.069259012016);
	vector->setHalf(true);
	vector->setTop(true);
	vector->setDark(true);
	vector->setDualOption(true);
	vector->setTripleOption(false);
	vector->setOption1("VECTOR");
	vector->setOption2("ON");
	vector->setChoice2("OFF");
	vector->on = SHOW_VECTORS;
	BUTTONS.push_back(vector);
	TopButton* vector2 = new TopButton(TOP_TYPE::VECTOR2_BTN);
	vector2->setIndex(7);
	vector2->setWidth(0.069259012016);
	vector2->setHalf(true);
	vector2->setTop(false);
	vector2->setDark(true);
	vector2->setDualOption(true);
	vector2->setTripleOption(false);
	vector2->setOption1("VECTOR");
	vector2->setOption2(std::to_string(vector2->value = vector_length));
	BUTTONS.push_back(vector2);
	TopButton* tmpdata = new TopButton(TOP_TYPE::TEMP_BTN);
	tmpdata->setIndex(8);
	tmpdata->setWidth(0.069259012016);
	tmpdata->setHalf(true);
	tmpdata->setTop(true);
	tmpdata->setDark(false);
	tmpdata->setDualOption(true);
	tmpdata->setTripleOption(false);
	tmpdata->setOption1("TEMP");
	tmpdata->setOption2("DATA");
	BUTTONS.push_back(tmpdata);
	TopButton* ldr = new TopButton(TOP_TYPE::LDR_BTN);
	ldr->setIndex(8);
	ldr->setWidth(0.069259012016);
	ldr->setHalf(true);
	ldr->setTop(false);
	ldr->setDark(true);
	ldr->setDualOption(true);
	ldr->setTripleOption(false);
	ldr->setOption1("LDR LNG");
	ldr->setOption2("1");
	BUTTONS.push_back(ldr);
	TopButton* local = new TopButton(TOP_TYPE::LOCAL_BTN);
	local->setIndex(9);
	local->setWidth(0.069259012016);
	local->setHalf(true);
	local->setTop(true);
	local->setDark(false);
	local->setDualOption(true);
	local->setTripleOption(false);
	local->setOption1("LOCAL");
	local->setOption2("101-188");
	BUTTONS.push_back(local);
	TopButton* local2 = new TopButton(TOP_TYPE::LOCAL2_BTN);
	local2->setIndex(9);
	local2->setWidth(0.069259012016);
	local2->setHalf(true);
	local2->setTop(false);
	local2->setDark(false);
	local2->setDualOption(true);
	local2->setTripleOption(false);
	local2->setOption1("LOCAL");
	local2->setOption2("189-276");
	BUTTONS.push_back(local2);
	TopButton* dbarea = new TopButton(TOP_TYPE::DB_AREA_BTN);
	dbarea->setIndex(10);
	dbarea->setWidth(0.069259012016);
	dbarea->setHalf(true);
	dbarea->setTop(true);
	dbarea->setDark(false);
	dbarea->setDualOption(true);
	dbarea->setTripleOption(false);
	dbarea->setOption1("DB");
	dbarea->setOption2("AREA");
	BUTTONS.push_back(dbarea);
	TopButton* dbedit = new TopButton(TOP_TYPE::DB_EDIT);
	dbedit->setIndex(10);
	dbedit->setWidth(0.069259012016);
	dbedit->setHalf(true);
	dbedit->setTop(false);
	dbedit->setDark(false);
	dbedit->setDualOption(false);
	dbedit->setTripleOption(false);
	dbedit->setOption1("DB EDIT");
	BUTTONS.push_back(dbedit);
	TopButton* db = new TopButton(TOP_TYPE::DB_BTN);
	db->setIndex(11);
	db->setWidth(0.069259012016);
	db->setHalf(true);
	db->setTop(true);
	db->setDark(true);
	db->setDualOption(true);
	db->setTripleOption(false);
	db->setOption1("DB");
	db->setOption2("ON");
	db->setChoice2("OFF");
	db->on = DB_BLOCK;
	BUTTONS.push_back(db);
	TopButton* initcntl = new TopButton(TOP_TYPE::INITCNTL);
	initcntl->setIndex(11);
	initcntl->setWidth(0.069259012016);
	initcntl->setHalf(true);
	initcntl->setTop(false);
	initcntl->setDark(true);
	initcntl->setDualOption(true);
	initcntl->setTripleOption(false);
	initcntl->setOption1("INIT");
	initcntl->setOption2("CNTL");
	BUTTONS.push_back(initcntl);
	TopButton* trksusp = new TopButton(TOP_TYPE::TRKSUP);
	trksusp->setIndex(12);
	trksusp->setWidth(0.069259012016);
	trksusp->setHalf(true);
	trksusp->setTop(true);
	trksusp->setDark(true);
	trksusp->setDualOption(true);
	trksusp->setTripleOption(false);
	trksusp->setOption1("TRK");
	trksusp->setOption2("SUSP");
	BUTTONS.push_back(trksusp);
	TopButton* term = new TopButton(TOP_TYPE::TERM_BTN);
	term->setIndex(12);
	term->setWidth(0.069259012016);
	term->setHalf(true);
	term->setTop(false);
	term->setDark(true);
	term->setDualOption(true);
	term->setTripleOption(false);
	term->setOption1("TERM");
	term->setOption2("CNTL");
	BUTTONS.push_back(term);
	TopButton* dcb = new TopButton(TOP_TYPE::DCB_BTN);
	dcb->setIndex(13);
	dcb->setWidth(0.069259012016);
	dcb->setHalf(true);
	dcb->setTop(true);
	dcb->setDark(true);
	dcb->setDualOption(true);
	dcb->setTripleOption(false);
	dcb->setOption1("DCB");
	dcb->setOption2("ON");
	dcb->setChoice2("OFF");
	dcb->on = true;
	BUTTONS.push_back(dcb);
	TopButton* oper = new TopButton(TOP_TYPE::OPER_BTN);
	oper->setIndex(13);
	oper->setWidth(0.069259012016);
	oper->setHalf(true);
	oper->setTop(false);
	oper->setDark(false);
	oper->setDualOption(true);
	oper->setTripleOption(false);
	oper->setOption1("OPER");
	oper->setOption2("MODE");
	BUTTONS.push_back(oper);
}

TopButton::TopButton(TOP_TYPE type) : params{ 0, 0, 0, 0 }
{
	this->type = type;
}

int TopButton::getIndex() {
	return TopButton::index;
}

void TopButton::setIndex(int value) {
	TopButton::index = value;
}

double TopButton::getWidth() {
	return TopButton::width;
}

void TopButton::setWidth(double value) {
	TopButton::width = value;
}

bool TopButton::getIsHalf() {
	return TopButton::half;
}

void TopButton::setHalf(bool value) {
	TopButton::half = value;
}

bool TopButton::getIsTop() {
	return TopButton::top;
}

void TopButton::setTop(bool value) {
	TopButton::top = value;
}

bool TopButton::getIsDark() {
	return TopButton::dark;
}

void TopButton::setDark(bool value) {
	TopButton::dark = value;
}

bool TopButton::isDualOption() {
	return TopButton::dual_option;
}

void TopButton::setDualOption(bool value) {
	TopButton::dual_option = value;
}

bool TopButton::isTripleOption() {
	return TopButton::triple_option;
}

void TopButton::setTripleOption(bool value) {
	TopButton::triple_option = value;
}

std::string TopButton::getOption1() {
	return TopButton::option1;
}

void TopButton::setOption1(std::string value) {
	TopButton::option1 = value;
}

std::string TopButton::getChoice1() {
	return TopButton::choice1;
}

void TopButton::setChoice1(std::string value) {
	TopButton::choice1 = value;
}

std::string TopButton::getOption2() {
	return TopButton::option2;
}

void TopButton::setOption2(std::string value) {
	TopButton::option2 = value;
}

std::string TopButton::getChoice2() {
	return TopButton::choice2;
}

void TopButton::setChoice2(std::string value) {
	TopButton::choice2 = value;
}


std::string TopButton::getOption3() {
	return TopButton::option3;
}

void TopButton::setOption3(std::string value) {
	TopButton::option3 = value;
}

int TopButton::handle()
{
	switch (this->type)
	{
	case TOP_TYPE::ROTATE_BTN:
	{
		rotation = this->value = 0;
		if (rotation != 0) 
		{ 
			rotateb->setDualOption(true);
			renderButtons = true;
		}
		else 
		{
			rotateb->setDualOption(false);
			renderButtons = true;
		}
		return 0;
	}
	case TOP_TYPE::DB_BTN:
	{
		DB_BLOCK = !DB_BLOCK;
		return 1;
	}
	case TOP_TYPE::DAY_NITE_BTN:
	{
		DAY = !DAY;
		renderSectorColours = true;
		return 1;
	}
	case TOP_TYPE::VECTOR_BTN:
	{
		SHOW_VECTORS = !SHOW_VECTORS;
		if (SHOW_VECTORS)
		{
			updateFlags[GBL_VECTOR] = true;
		}
		return 1;
	}
	default:
		break;
	}
	return 0;
}

int TopButton::handleScroll(bool hi_scroll)
{
	switch (this->type)
	{
	case TOP_TYPE::RANGE_BTN:
	{
		int change = this->value;
		if (zoom_phase < 2)
			return 0;
		if (hi_scroll)
		{
			if (!((this->value + 1) > 600)) {
				this->value += 1;
			}
		}
		else
		{
			if (!((this->value - 1) < 3)) {
				this->value -= 1;
			}
		}

		if (zoom_phase == 2) {
			//hi_scroll ? r_aircraft_size -= (r_aircraft_size * 0.1) : r_aircraft_size += (r_aircraft_size * 0.1);
			//hi_scroll ? h_aircraft_size -= (h_aircraft_size * 0.1) : h_aircraft_size += (h_aircraft_size * 0.1);
		}

		if (this->value != change) {
			this->setOption2(std::to_string(range = this->value));//range is set in 100's of feet so 100 = 10,000 feet
			mZoom = zoom_from_range();
			updateFlags[GBL_COLLISION_LINE] = true;
		}
		return 1;
	}
	case TOP_TYPE::ROTATE_BTN:
	{
		int change = this->value;
		if (hi_scroll)
		{
			if ((this->value) + 1 >= 360)
				this->value = 0;
			else
				this->value += 1;
		}
		else
		{
			if ((this->value - 1) < 0)
				this->value = 359;
			else
				this->value -= 1;
		}
		rotation = this->value;
		if (this->value != change) {
			this->setOption2(std::to_string((int) (rotation = this->value)));//range is set in 100's of feet so 100 = 10,000 feet
		}
		rotation != 0 ? rotateb->setDualOption(true) : rotateb->setDualOption(false);
		return 1;
	}
	case TOP_TYPE::VECTOR2_BTN:
	{
		if (hi_scroll)
		{
			if (!((this->value + 1) > 20))
				this->value += 1;
		}
		else
		{
			if (!((this->value - 1) < 1))
				this->value -= 1;
		}
		this->setOption2(std::to_string(this->value));
		vector_length = this->value;
		if (SHOW_VECTORS)
		{
			updateFlags[GBL_VECTOR] = true;
		}
		return 1;
	}
	default:
		break;
	}
	return 0;
}

void TopButton::refreshOption2()
{
	switch (this->type)
	{
	case TOP_TYPE::RANGE_BTN:
	{
		if (zoom_phase < 2)
			this->setOption2("NSET");
		else
			this->setOption2(std::to_string(this->value = range));
		break;
	}
	default:
	{
		this->setOption2(std::to_string(this->value));
		break;
	}
	}
	renderButtons = true;
}
