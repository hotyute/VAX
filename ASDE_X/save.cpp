#include "save.h"

#include <fstream>
#include <boost/dll.hpp>

#include "usermanager.h"
#include "basic_stream.h"
#include "interfaces.h" // For sendSystemMessage
#include "filereader.h"
// #include "comms.h" // REMOVE THIS
#include "user.h" // For UserData::NUM_SAVED_COMMS_LINES and CommsLinePersistentData

void save_info()
{
	if (!USER) return; // Cannot save if USER is not initialized

	BasicStream buf = BasicStream(512); // Increased buffer size slightly

	// Save Identity (Opcode 1) - NO CHANGE
	buf.create_frame_var_size_word(1);
	buf.write_string(USER->getIdentity()->callsign.c_str());
	buf.write_string(USER->getIdentity()->login_name.c_str());
	buf.write_string(USER->getIdentity()->username.c_str());
	buf.write_string(USER->getIdentity()->password.c_str());
	buf.write_byte(USER->getIdentity()->controller_rating);
	buf.write_byte(static_cast<int>(USER->getIdentity()->controller_position));
	buf.end_frame_var_size_word();

	// Save Paths (Opcode 2) - NO CHANGE
	buf.create_frame_var_size_word(2);
	buf.write_string(LAST_ADX_PATH.c_str());
	buf.write_string(LAST_CPF_PATH.c_str());
	buf.write_string(LAST_POF_PATH.c_str());
	buf.write_string(LAST_ALIAS_PATH.c_str());
	buf.end_frame_var_size_word();

	// --- NEW Comms Configuration Saving (Opcode 3) ---
	buf.create_frame_var_size_word(3);
	buf.write_byte(UserData::NUM_SAVED_COMMS_LINES); // Number of lines being saved
	buf.write_byte(static_cast<uint8_t>(USER->userdata.primaryCommsLineIndex)); // Save primary index (-1 if none)

	for (int i = 0; i < UserData::NUM_SAVED_COMMS_LINES; ++i) {
		const auto& lineConfig = USER->userdata.commsConfig[i];
		// buf.write_byte(i); // Index not strictly needed if always saving all lines in order
		buf.write_string(lineConfig.pos.c_str());
		buf.write_string(lineConfig.freq.c_str());
		buf.write_byte(lineConfig.tx_checked ? 1 : 0);
		buf.write_byte(lineConfig.rx_checked ? 1 : 0);
		buf.write_byte(lineConfig.hdst_checked ? 1 : 0);
		buf.write_byte(lineConfig.spkr_checked ? 1 : 0);
	}
	buf.end_frame_var_size_word();
	// --- END NEW Comms Configuration Saving ---

	const auto full_path = boost::dll::program_location().parent_path();
	std::string data_file_path = (full_path / "data.bin").string(); // Use boost::filesystem path concatenation

	std::fstream myFile(data_file_path, std::ios::out | std::ios::binary | std::ios::trunc); // Truncate to overwrite
	if (myFile.is_open()) {
		myFile.write(buf.data, buf.index);
		myFile.close();
		sendSystemMessage("Data Saved.");
	}
	else {
		sendErrorMessage("Failed to open data.bin for saving.");
	}
}

void read_info()
{
	if (!USER) return; // Cannot load if USER is not initialized

	auto full_path = boost::dll::program_location().parent_path();
	std::string data_file_path = (full_path / "data.bin").string();

	std::fstream ifs(data_file_path, std::ios::in | std::ios::binary | std::ios::ate);

	if (ifs.is_open())
	{
		std::fstream::pos_type size = ifs.tellg();
		if (size == 0) {
			ifs.close();
			sendSystemMessage("data.bin is empty. No data loaded.");
			return;
		}


		BasicStream buf = BasicStream(size);
		ifs.seekg(0, std::ios::beg);
		ifs.read(buf.data, size); // Read directly into buffer, index starts at 0
		ifs.close();

		// buf.index is 0 here, data is loaded up to 'size'.
		// We need to process the buffer by advancing buf.index correctly.
		// The original read_info had buf.index set to the size, which seems incorrect for reading.
		// Let's assume buf.data is filled, and we read from buf.index = 0.

		while (buf.available() > 0) // Check if there's data left to read
		{
			if (buf.available() < 1) break; // Not enough for opcode
			int opcode = buf.read_unsigned_byte();
			if (buf.available() < 2 && (opcode == 1 || opcode == 2 || opcode == 3)) break; // Not enough for size short

			if (opcode == 1) // Identity
			{
				int frame_size = buf.read_unsigned_short();
				if (buf.available() < frame_size) { sendErrorMessage("data.bin corrupt (opcode 1)"); break; }
				Identity& id = *USER->getIdentity();
				id.callsign = buf.read_std_string(); // Use read_std_string for safety
				id.login_name = buf.read_std_string();
				id.username = buf.read_std_string();
				id.password = buf.read_std_string();
				id.controller_rating = buf.read_unsigned_byte();
				id.controller_position = static_cast<POSITIONS>(buf.read_unsigned_byte());
			}
			else if (opcode == 2) // Paths
			{
				int frame_size = buf.read_unsigned_short();
				if (buf.available() < frame_size) { sendErrorMessage("data.bin corrupt (opcode 2)"); break; }
				// if (frame_size > 0) // This check was problematic as frame_size could be 0 for empty strings
				// {
				LAST_ADX_PATH = buf.read_std_string();
				LAST_CPF_PATH = buf.read_std_string();
				LAST_POF_PATH = buf.read_std_string();
				LAST_ALIAS_PATH = buf.read_std_string();

				if (!LAST_ADX_PATH.empty()) open_adx(LAST_ADX_PATH);
				if (!LAST_CPF_PATH.empty()) parseCpfFile(LAST_CPF_PATH, filerdr.collisionPaths);
				// }
			}
			else if (opcode == 3) // Comms Config
			{
				int frame_size = buf.read_unsigned_short();
				if (buf.available() < frame_size) { sendErrorMessage("data.bin corrupt (opcode 3)"); break; }

				uint8_t numLinesSaved = buf.read_unsigned_byte();
				USER->userdata.primaryCommsLineIndex = static_cast<int8_t>(buf.read_unsigned_byte()); // Read as signed char for -1

				for (int i = 0; i < numLinesSaved; ++i) {
					auto& lineConfig = USER->userdata.commsConfig[i];
					lineConfig.pos = buf.read_std_string();
					lineConfig.freq = buf.read_std_string();
					lineConfig.tx_checked = (buf.read_unsigned_byte() == 1);
					lineConfig.rx_checked = (buf.read_unsigned_byte() == 1);
					lineConfig.hdst_checked = (buf.read_unsigned_byte() == 1);
					lineConfig.spkr_checked = (buf.read_unsigned_byte() == 1);
				}
			}
			else
			{
				sendErrorMessage("Unknown opcode in data.bin: " + std::to_string(opcode));
				break; // Stop processing on unknown opcode
			}
		}
		sendSystemMessage("Data Loaded.");
	}
	else {
		sendSystemMessage("Failed to load Data (data.bin not found or could not be opened).");
	}
}