#include "save.h"

#include <fstream>

#include "usermanager.h"
#include "Stream.h"

void save_info()
{
    Stream buf = Stream(256);

    buf.createFrameVarSizeWord(1);
    buf.writeString((char*)USER->getIdentity()->callsign.c_str());
    buf.endFrameVarSizeWord();

    std::fstream myFile("data.bin", std::ios::out | std::ios::binary);
    myFile.write(buf.buffer, buf.writeIndex);
    myFile.close();
}

void read_info()
{
    std::fstream ifs("data.bin", std::ios::in | std::ios::binary | std::ios::ate);

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
                char callSign1[1024], full_name[1024], username[1024];
                buf.readString(callSign1);
            }
        }
        //do something with data

    }
}
