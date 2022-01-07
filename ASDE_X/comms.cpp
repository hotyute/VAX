#include "comms.h"

CommsLine* comms_line0 = nullptr, * comms_line1 = nullptr;
CommsLine* cur_prime = nullptr;
std::vector<CommsLine*> COMMS_MAP(100, nullptr);

void RenderCommunications(bool open, double x_, double y_, bool refresh, bool expand)
{
	if (expand && communications)
	{
		double width = 280, x = x_ == -1 ? (CLIENT_WIDTH / 2.0) - (width / 2.0) : x_;
		double height = expand ? 550 : 450, y = y_ == -1 ? (CLIENT_HEIGHT / 2.0) - (height / 2.0) : y_;
		communications->UpdatePane1(x, width, y, height);
	}
	else {
		communications = new InterfaceFrame(COMMS_INTERFACE);
		std::fill(COMMS_MAP.begin(), COMMS_MAP.end(), nullptr);

		communications->title = "COMMUNICATIONS";

		double width = 280, x = x_ == -1 ? (CLIENT_WIDTH / 2.0) - (width / 2.0) : x_;
		double height = expand ? 550 : 450, y = y_ == -1 ? (CLIENT_HEIGHT / 2.0) - (height / 2.0) : y_;
		communications->Pane1(x, width, y, height);

		double spacing_x = 0.15, spacing_y = 0.07;
		double start_x = 0.6, start_y = 0.03;
		const double bar_offset = 0.01;

		double radio_sep = 0.0, label_sep = 0.0;
		const double _radsep_x = 30.0;

		Label* prim_label = new Label(communications, "PRIM", x + (width - (width * ((start_x + 0.60) - spacing_x))),
			50.0, 10.0, y + (height - (height * (start_y + spacing_y))), 20, -10.0);
		prim_label->centered = 1;
		communications->children[prim_label->index = PRIM_LABEL] = prim_label;

		label_sep += (0.24 * width);

		Label* name_label = new Label(communications, "Name", x + (width - (width * ((start_x + 0.60) - spacing_x))) + label_sep,
			50.0, 10.0, y + (height - (height * (start_y + spacing_y))), 20, -10.0);
		name_label->centered = 1;
		communications->children[name_label->index = NAME_LABEL] = name_label;

		label_sep += (0.25 * width);

		Label* tx_label = new Label(communications, "TX", x + (width - (width * ((start_x + 0.60) - spacing_x))) + label_sep,
			50.0, 10.0, y + (height - (height * (start_y + spacing_y))), 20, -10.0);
		tx_label->centered = 1;
		communications->children[tx_label->index = TX_LABEL] = tx_label;

		label_sep += (_radsep_x + 2.0);

		Label* rx_label = new Label(communications, "RX", x + (width - (width * ((start_x + 0.60) - spacing_x))) + label_sep,
			50.0, 10.0, y + (height - (height * (start_y + spacing_y))), 20, -10.0);
		rx_label->centered = 1;
		communications->children[rx_label->index = RX_LABEL] = rx_label;

		label_sep += _radsep_x;

		Label* hdst_label = new Label(communications, "HDST", x + (width - (width * ((start_x + 0.60) - spacing_x))) + label_sep,
			50.0, 10.0, y + (height - (height * (start_y + spacing_y))), 20, -10.0);
		hdst_label->centered = 1;
		communications->children[hdst_label->index = HDST_LABEL] = hdst_label;

		label_sep += (_radsep_x + 2.0);

		Label* spkr_label = new Label(communications, "SPKR", x + (width - (width * ((start_x + 0.60) - spacing_x))) + label_sep,
			50.0, 10.0, y + (height - (height * (start_y + spacing_y))), 20, -10.0);
		spkr_label->centered = 1;
		communications->children[spkr_label->index = SPKR_LABEL] = spkr_label;

		radio_sep = 0.0;
		start_y = 0.1;

		ClickButton* slot_0;
		Radio* prim_0, * radio1_0, * radio1_1, * radio1_2, * radio1_3;

		prim_0 = new Radio(communications, x + (width - (width * ((start_x + 0.5) - spacing_x))),
			RADIO_SIZE, y + (height - (height * (start_y + spacing_y))), RADIO_SIZE);

		slot_0 = new ClickButton(communications, "----", x + (width - (width * ((start_x + 0.4) - spacing_x))),
			90.0, y + (height - (height * ((start_y + bar_offset) + spacing_y))), 25.0);

		radio1_0 = new Radio(communications, x + (width - (width * (start_x - spacing_x))),
			RADIO_SIZE, y + (height - (height * (start_y + spacing_y))), RADIO_SIZE);

		radio_sep += _radsep_x;
		radio1_1 = new Radio(communications, x + (width - (width * (start_x - spacing_x))) + radio_sep,
			RADIO_SIZE, y + (height - (height * (start_y + spacing_y))), RADIO_SIZE);

		radio_sep += _radsep_x;
		radio1_2 = new Radio(communications, x + (width - (width * (start_x - spacing_x))) + radio_sep,
			RADIO_SIZE, y + (height - (height * (start_y + spacing_y))), RADIO_SIZE);

		radio_sep += _radsep_x;
		radio1_3 = new Radio(communications, x + (width - (width * (start_x - spacing_x))) + radio_sep,
			RADIO_SIZE, y + (height - (height * (start_y + spacing_y))), RADIO_SIZE);

		communications->children[prim_0->index = PRIM_0] = prim_0;


		communications->children[slot_0->index = BUTTON_0] = slot_0;
		slot_0->btn_border = false;


		communications->children[radio1_0->index = RADIOLINE1_0] = radio1_0;

		communications->children[radio1_1->index = RADIOLINE1_1] = radio1_1;

		communications->children[radio1_2->index = RADIOLINE1_2] = radio1_2;

		communications->children[radio1_3->index = RADIOLINE1_3] = radio1_3;

		if (comms_line0)
		{
			delete comms_line0;
			comms_line0 = nullptr;
		}
		comms_line0 = new CommsLine(prim_0, slot_0, radio1_0, radio1_1, radio1_2, radio1_3);
		append_comms(PRIM_0, BUTTON_0, RADIOLINE1_0, RADIOLINE1_1, RADIOLINE1_2, RADIOLINE1_3, comms_line0);

		//2nd line

		radio_sep = 0.0;
		start_y += spacing_y;

		ClickButton* slot_1;
		Radio* prim_1, * radio2_0, * radio2_1, * radio2_2, * radio2_3;
		prim_1 = new Radio(communications, x + (width - (width * ((start_x + 0.5) - spacing_x))),
			RADIO_SIZE, y + (height - (height * (start_y + spacing_y))), RADIO_SIZE);

		slot_1 = new ClickButton(communications, "----", x + (width - (width * ((start_x + 0.4) - spacing_x))),
			90.0, y + (height - (height * ((start_y + bar_offset) + spacing_y))), 25.0);

		radio2_0 = new Radio(communications, x + (width - (width * (start_x - spacing_x))),
			RADIO_SIZE, y + (height - (height * (start_y + spacing_y))), RADIO_SIZE);

		radio_sep += _radsep_x;
		radio2_1 = new Radio(communications, x + (width - (width * (start_x - spacing_x))) + radio_sep,
			RADIO_SIZE, y + (height - (height * (start_y + spacing_y))), RADIO_SIZE);

		radio_sep += _radsep_x;
		radio2_2 = new Radio(communications, x + (width - (width * (start_x - spacing_x))) + radio_sep,
			RADIO_SIZE, y + (height - (height * (start_y + spacing_y))), RADIO_SIZE);

		radio_sep += _radsep_x;
		radio2_3 = new Radio(communications, x + (width - (width * (start_x - spacing_x))) + radio_sep,
			RADIO_SIZE, y + (height - (height * (start_y + spacing_y))), RADIO_SIZE);

		communications->children[prim_1->index = PRIM_1] = prim_1;

		communications->children[slot_1->index = BUTTON_1] = slot_1;
		slot_1->btn_border = false;

		communications->children[radio2_0->index = RADIOLINE2_0] = radio2_0;

		communications->children[radio2_1->index = RADIOLINE2_1] = radio2_1;

		communications->children[radio2_2->index = RADIOLINE2_2] = radio2_2;

		communications->children[radio2_3->index = RADIOLINE2_3] = radio2_3;

		if (comms_line1)
		{
			delete comms_line1;
			comms_line1 = nullptr;
		}
		comms_line1 = new CommsLine(prim_1, slot_1, radio2_0, radio2_1, radio2_2, radio2_3);
		append_comms(PRIM_1, BUTTON_1, RADIOLINE2_0, RADIOLINE2_1, RADIOLINE2_2, RADIOLINE2_3, comms_line1);

		CloseButton* closeb = new CloseButton(communications, 15, 15);
		communications->children[closeb->index = COMMS_CLOSE] = closeb;
	}

	if (open)
	{
		communications->doOpen(true, true);//delete's old object while opening, this should be before setting frame vector
	}
	else
	{
		communications->doClose();
	}

	communications->doInsert();
}

void append_comms(int prim, int btn, int tx, int rx, int hdst, int spkr, CommsLine* comms)
{
	COMMS_MAP[prim] = comms;
	COMMS_MAP[btn] = comms;
	COMMS_MAP[tx] = comms;
	COMMS_MAP[rx] = comms;
	COMMS_MAP[hdst] = comms;
	COMMS_MAP[spkr] = comms;
}

void prime_comms(CommsLine* line)
{
	if (cur_prime != line)
	{
		if (cur_prime && cur_prime->prim && cur_prime->prim->checked)
		{
			cur_prime->prim->toggle();
			cur_prime = nullptr;
		}
		if (line->prim && !line->prim->checked)
		{
			line->prim->toggle();
			cur_prime = line;
		}
	}
	else {
		if (line->prim->checked)
			line->prim->toggle();
		cur_prime = nullptr;
	}
}