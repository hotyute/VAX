#include "save.h"

#include <fstream>
#include <boost/dll.hpp>

#include "usermanager.h"
#include "Stream.h"
#include "interfaces.h"

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
        }

        sendSystemMessage("Data Loaded.");
        //do something with data
        return;
    }

    sendSystemMessage("Failed to load Data.");
}
