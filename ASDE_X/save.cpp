#include "save.h"

#include <fstream>
#include <boost/dll.hpp>

#include "gui/ui_windows/settings_window.h"
#include "usermanager.h"
#include "basic_stream.h"
#include "interfaces.h" 
#include "filereader.h"
#include "user.h" 

void save_info()
{
	if (!USER) return;

	BasicStream buf = BasicStream(512);

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

	// Comms Configuration Saving (Opcode 3) - NO CHANGE FROM PREVIOUS
	buf.create_frame_var_size_word(3);
	buf.write_byte(UserData::NUM_SAVED_COMMS_LINES);
	buf.write_byte(static_cast<uint8_t>(USER->userdata.primaryCommsLineIndex));

	for (int i = 0; i < UserData::NUM_SAVED_COMMS_LINES; ++i) {
		const auto& lineConfig = USER->userdata.commsConfig[i];
		buf.write_string(lineConfig.pos.c_str());
		buf.write_string(lineConfig.freq.c_str());
		buf.write_byte(lineConfig.tx_checked ? 1 : 0);
		buf.write_byte(lineConfig.rx_checked ? 1 : 0);
		buf.write_byte(lineConfig.hdst_checked ? 1 : 0);
		buf.write_byte(lineConfig.spkr_checked ? 1 : 0);
	}
	buf.end_frame_var_size_word();

	// --- General App Settings (Opcode 4) ---
	buf.create_frame_var_size_word(4); // Using Opcode 4
	buf.write_byte(USER->userdata.alwaysOnTop ? 1 : 0);
	// Future general settings can be added here.
	// buf.write_float(USER->userdata.window_opacity); // If opacity persistence is added
	buf.end_frame_var_size_word();
	// --- END General App Settings ---

	const auto full_path = boost::dll::program_location().parent_path();
	std::string data_file_path = (full_path / "data.bin").string();

	std::fstream myFile(data_file_path, std::ios::out | std::ios::binary | std::ios::trunc);
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
	if (!USER) return;

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
		ifs.read(buf.data, size);
		ifs.close();

		bool alwaysOnTopRead = false; // Flag to ensure it's applied once after reading

		while (buf.available() > 0)
		{
			if (buf.available() < 1) break;
			int opcode = buf.read_unsigned_byte();
			// For opcodes 1,2,3,4 check if enough bytes for size short
			if (buf.available() < 2 && (opcode >= 1 && opcode <= 4)) break;

			if (opcode == 1)
			{
				int frame_size = buf.read_unsigned_short();
				if (buf.available() < frame_size) { sendErrorMessage("data.bin corrupt (opcode 1)"); break; }
				Identity& id = *USER->getIdentity();
				id.callsign = buf.read_std_string();
				id.login_name = buf.read_std_string();
				id.username = buf.read_std_string();
				id.password = buf.read_std_string();
				id.controller_rating = buf.read_unsigned_byte();
				id.controller_position = static_cast<POSITIONS>(buf.read_unsigned_byte());
			}
			else if (opcode == 2)
			{
				int frame_size = buf.read_unsigned_short();
				if (buf.available() < frame_size) { sendErrorMessage("data.bin corrupt (opcode 2)"); break; }
				LAST_ADX_PATH = buf.read_std_string();
				LAST_CPF_PATH = buf.read_std_string();
				LAST_POF_PATH = buf.read_std_string();
				LAST_ALIAS_PATH = buf.read_std_string();

				if (!LAST_ADX_PATH.empty()) open_adx(LAST_ADX_PATH);
				if (!LAST_CPF_PATH.empty()) parseCpfFile(LAST_CPF_PATH, filerdr.collisionPaths);
			}
			else if (opcode == 3)
			{
				int frame_size = buf.read_unsigned_short();
				if (buf.available() < frame_size) { sendErrorMessage("data.bin corrupt (opcode 3)"); break; }

				uint8_t numLinesSaved = buf.read_unsigned_byte();
				USER->userdata.primaryCommsLineIndex = static_cast<int8_t>(buf.read_unsigned_byte());

				for (int i = 0; i < numLinesSaved; ++i) {
					if (i < UserData::NUM_SAVED_COMMS_LINES) { // Bounds check
						auto& lineConfig = USER->userdata.commsConfig[i];
						lineConfig.pos = buf.read_std_string();
						lineConfig.freq = buf.read_std_string();
						lineConfig.tx_checked = (buf.read_unsigned_byte() == 1);
						lineConfig.rx_checked = (buf.read_unsigned_byte() == 1);
						lineConfig.hdst_checked = (buf.read_unsigned_byte() == 1);
						lineConfig.spkr_checked = (buf.read_unsigned_byte() == 1);
					}
					else { // Saved data has more lines than current config, skip them
						buf.read_std_string(); // pos
						buf.read_std_string(); // freq
						buf.read_unsigned_byte(); // tx
						buf.read_unsigned_byte(); // rx
						buf.read_unsigned_byte(); // hdst
						buf.read_unsigned_byte(); // spkr
					}
				}
			}
			else if (opcode == 4) // General App Settings
			{
				int frame_size = buf.read_unsigned_short();
				if (buf.available() < frame_size) { sendErrorMessage("data.bin corrupt (opcode 4)"); break; }

				USER->userdata.alwaysOnTop = (buf.read_unsigned_byte() == 1);
				alwaysOnTopRead = true;
				// Read future general settings here:
				// USER->userdata.window_opacity = buf.read_float(); // If opacity persistence is added
			}
			else
			{
				sendErrorMessage("Unknown opcode in data.bin: " + std::to_string(opcode));
				break;
			}
		}
		sendSystemMessage("Data Loaded.");

		// Apply settings that need to be active at startup
		if (alwaysOnTopRead) { // Only apply if it was actually read from the file
			SettingsWindow::SetGlobalAlwaysOnTop(USER->userdata.alwaysOnTop);
		}
		// If opacity is persisted:
		// if (windowOpacityRead) SettingsWindow::SetGlobalWindowOpacity(USER->userdata.window_opacity);

	}
	else {
		sendSystemMessage("Failed to load Data (data.bin not found or could not be opened).");
		// Apply default settings if file not found
		SettingsWindow::SetGlobalAlwaysOnTop(false); // Default off
		// SettingsWindow::SetGlobalWindowOpacity(0.8f); // Default opacity
	}
}