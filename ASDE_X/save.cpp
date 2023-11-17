#include "save.h"

#include <fstream>
#include <boost/dll.hpp>

#include "usermanager.h"
#include "basic_stream.h"
#include "interfaces.h"
#include "filereader.h"
#include "comms.h"

void save_info()
{
	BasicStream buf = BasicStream(256);

	buf.create_frame_var_size_word(1);
	buf.write_string(USER->getIdentity()->callsign.c_str());
	buf.write_string(USER->getIdentity()->login_name.c_str());
	buf.write_string(USER->getIdentity()->username.c_str());
	buf.write_string(USER->getIdentity()->password.c_str());
	buf.write_byte(USER->getIdentity()->controller_rating);
	buf.write_byte(static_cast<int>(USER->getIdentity()->controller_position));
	buf.end_frame_var_size_word();

	buf.create_frame_var_size_word(2);
	buf.write_string(LAST_ADX_PATH.c_str());
	buf.write_string(LAST_CPF_PATH.c_str());
	buf.write_string(LAST_POF_PATH.c_str());
	buf.write_string(LAST_ALIAS_PATH.c_str());
	buf.end_frame_var_size_word();

	if (comms_line0 && comms_line1)
	{
		buf.create_frame_var_size_word(3);
		buf.write_byte(2);

		buf.write_byte(0);
		buf.write_string(comms_line0->pos.c_str());
		buf.write_string(comms_line0->freq.c_str());
		buf.write_byte(comms_line0->tx->checked ? 1 : 0);
		buf.write_byte(comms_line0->rx->checked ? 1 : 0);
		buf.write_byte(comms_line0->hdst->checked ? 1 : 0);
		buf.write_byte(comms_line0->spkr->checked ? 1 : 0);

		buf.write_byte(1);
		buf.write_string(comms_line1->pos.c_str());
		buf.write_string(comms_line1->freq.c_str());
		buf.write_byte(comms_line1->tx->checked ? 1 : 0);
		buf.write_byte(comms_line1->rx->checked ? 1 : 0);
		buf.write_byte(comms_line1->hdst->checked ? 1 : 0);
		buf.write_byte(comms_line1->spkr->checked ? 1 : 0);

		buf.end_frame_var_size_word();
	}

	const auto full_path = boost::dll::program_location().parent_path();

	std::fstream myFile(full_path.string() + "\\data.bin", std::ios::out | std::ios::binary);
	myFile.write(buf.data, buf.index);
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

		BasicStream buf = BasicStream(size);

		ifs.seekg(0, std::ios::beg);
		ifs.read(buf.data + buf.index, size);
		ifs.close();

		while (buf.available() != 0)
		{
			int opcode = buf.read_unsigned_byte();

			if (opcode == 1)
			{
				int size = buf.read_unsigned_short();
				Identity& id = *USER->getIdentity();
				id.callsign = buf.read_string();
				id.login_name = buf.read_string();
				id.username = buf.read_string();
				id.password = buf.read_string();


				USER->getIdentity()->controller_rating = buf.read_unsigned_byte();
				USER->getIdentity()->controller_position = static_cast<POSITIONS>(buf.read_unsigned_byte());
			}
			else if (opcode == 2)
			{
				int size = buf.read_unsigned_short();
				if (size > 0)
				{
					std::string file_path = buf.read_string();
					std::string cpf_file_path = buf.read_string();
					std::string pof_path = buf.read_string();
					std::string alias_path = buf.read_string();
					if (!file_path.empty())
						open_adx(file_path);
					if (!cpf_file_path.empty())
						parseCpfFile(cpf_file_path, filerdr.collisionPaths);
				}
			}
			else if (opcode == 3)
			{
				int size = buf.read_unsigned_short();
				if (size >= buf.available())
				{
					int length = buf.read_unsigned_byte();
					for (int i = 0; i < length; i++)
					{
						int id = buf.read_unsigned_byte();

						std::string pos = buf.read_string();
						std::string freq = buf.read_string();
						printf("%s", pos.c_str());;

						bool tx = buf.read_unsigned_byte() == 1;
						bool rx = buf.read_unsigned_byte() == 1;
						bool hdst = buf.read_unsigned_byte() == 1;
						bool spkr = buf.read_unsigned_byte() == 1;

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
