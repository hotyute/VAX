#ifndef COMMS_H
#define COMMS_H

#include "radio.h"
#include "interfaces.h"

void RenderCommunications(bool open, double x_, double y_, int expand);

#ifndef CommsLine_interfaces_h
#define CommsLine_interfaces_h
class CommsLine {
public:
	std::string pos = "", freq = "";
	ClickButton* btn;
	Radio* prim, * tx, * rx, * hdst, * spkr;
	CommsLine(Radio* _prim, ClickButton* _btn, Radio* _tx, Radio* _rx, Radio* _hdst, Radio* _spkr) : 
		prim(_prim), btn(_btn), tx(_tx), rx(_rx), hdst(_hdst), spkr(_spkr) { };
};
#endif

void append_comms(int prim, int btn, int tx, int rx, int hdst, int spkr, CommsLine* comms);
void prime_comms(CommsLine* line);

extern CommsLine* comms_line0, * comms_line1;
extern CommsLine* cur_prime, * cur_edit;
extern std::vector<CommsLine*> COMMS_MAP;
extern bool expanded;

#endif
