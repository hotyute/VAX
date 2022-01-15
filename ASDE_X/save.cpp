#include "save.h"

#include <fstream>
#include <boost/dll.hpp>

#include "usermanager.h"
#include "Stream.h"
#include "interfaces.h"
#include "filereader.h"
#include "comms.h"

void save_info()
{
	Stream buf = Stream(256);

	buf.createFrameVarSizeWord(1);
	buf.writeString((char*)USER->getIdentity()->callsign.c_str());
	buf.writeString((char*)USER->getIdentity()->login_name.c_str());
	buf.writeString((char*)USER->getIdentity()->username.c_str());
	buf.writeString((char*)USER->getIdentity()->password.c_str());
	buf.writeByte(USER->getIdentity()->controller_rating);
	buf.writeByte(static_cast<int>(USER->getIdentity()->controller_position));
	buf.endFrameVarSizeWord();

	buf.createFrameVarSizeWord(2);
	buf.writeString((char*)LAST_ADX_PATH.c_str());
	buf.writeString((char*)LAST_POF_PATH.c_str());
	buf.writeString((char*)LAST_ALIAS_PATH.c_str());
	buf.endFrameVarSizeWord();

	buf.createFrameVarSizeWord(3);
	buf.writeByte(2);

	buf.writeByte(0);
	buf.writeString((char*)comms_line0->pos.c_str());
	buf.writeString((char*)comms_line0->freq.c_str());
	buf.writeByte(comms_line0->tx->checked ? 1 : 0);
	buf.writeByte(comms_line0->rx->checked ? 1 : 0);
	buf.writeByte(comms_line0->hdst->checked ? 1 : 0);
	buf.writeByte(comms_line0->spkr->checked ? 1 : 0);

	buf.writeByte(1);
	buf.writeString((char*)comms_line1->pos.c_str());
	buf.writeString((char*)comms_line1->freq.c_str());
	buf.writeByte(comms_line1->tx->checked ? 1 : 0);
	buf.writeByte(comms_line1->rx->checked ? 1 : 0);
	buf.writeByte(comms_line1->hdst->checked ? 1 : 0);
	buf.writeByte(comms_line1->spkr->checked ? 1 : 0);

	buf.endFrameVarSizeWord();

	auto full_path = boost::dll::program_location().parent_path();

	std::fstream myFile(full_path.string() + "\\data.bin", std::ios::out | std::ios::binary);
	myFile.write(buf.buffer, buf.writeIndex);
	myFile.close();
	sendSystemMessage("Data Saved.");
}

void read_info()
{
	auto full_path = boost::dll::program_location().parent_path();

	std::fstream ifs(full_path.string() + "\\data.bin", std::ios::in | std::ios::binary | std::ios::ate);

	if (ifs.is_open())
	{
		std::fstream::pos_type size = ifs.tellg();

		Stream buf = Stream(size);

		ifs.seekg(0, std::ios::beg);
		ifs.read(buf.buffer + buf.writeIndex, size);
		buf.writeIndex += size;
		ifs.close();

		while (buf.remaining() != 0)
		{
			int opcode = buf.readUnsignedByte();

			if (opcode == 1)
			{
				int size = buf.readUnsignedWord();
				Identity& id = *USER->getIdentity();
				buf.readString(20, id.callsign);
				buf.readString(20, id.login_name);
				buf.readString(20, id.username);
				buf.readString(20, id.password);


				USER->getIdentity()->controller_rating = buf.readUnsignedByte();
				USER->getIdentity()->controller_position = static_cast<POSITIONS>(buf.readUnsignedByte());
			}
			else if (opcode == 2)
			{
				int size = buf.readUnsignedWord();
				if (size > 0)
				{
					std::string file_path, pof_path, alias_path;
					buf.readString(256, file_path);
					buf.readString(256, pof_path);
					buf.readString(256, alias_path);
					if (!file_path.empty())
					{
						open_adx(file_path);
					}
				}
			}
			else if (opcode == 3)
			{
				int size = buf.readUnsignedWord();
				printf("%d\n", size);
				if (size >= buf.remaining())
				{
					int length = buf.readUnsignedByte();
					for (int i = 0; i < length; i++)
					{
						int id = buf.readUnsignedByte();

						std::string pos, freq;

						bool tx, rx, hdst, spkr;
						buf.readString(256, pos);
						buf.readString(256, freq);
						tx = buf.readUnsignedByte() == 1;
						rx = buf.readUnsignedByte() == 1;
						hdst = buf.readUnsignedByte() == 1;
						spkr = buf.readUnsignedByte() == 1;

						CommsLine* line = nullptr;
						if (!communications)
						{
							RenderCommunications(false, -1, -1, 0);
						}
						switch (id)
						{
						case 0:
							line = comms_line0;
							break;
						case 1:
							line = comms_line1;
							break;
						}

						if (line)
						{
							line->pos = pos;
							line->btn->text = pos;
							line->freq = freq;
							line->tx->checked = tx;
							line->rx->checked = rx;
							line->hdst->checked = hdst;
							line->spkr->checked = spkr;

							renderDrawings = true;
						}
					}
				}
			}
		}

		sendSystemMessage("Data Loaded.");
		//do something with data
		return;
	}

	sendSystemMessage("Failed to load Data.");
}
