#ifndef STREAM_H
#define STREAM_H

#include <windows.h> //for everything extra
#include <stdlib.h> //for NULL (conversations s2i s2l s2ul s2d) rand srand system
#include <stdio.h>  //for printf()

static const int frameStackSize = 10;

class Stream {
	public:
		Stream();  //construct0r
		Stream(int newSize); //new size
		~Stream(); //destruct0r of all pointers
		void createFrame(int id);

		void createFrameVarSize(int id);
		void createFrameVarSizeWord(int id);
		void endFrameVarSize();
		void endFrameVarSizeWord();
		void writeByte(int i);
		void writeByteA(int i);
		void writeByteC(int i);
		void writeBytes(char abyte0[], int i, int j);
		void writeByteS(int i);
		void writeBytes_reverse(char abyte0[], int i, int j);
		void writeBytes_reverseA(char abyte0[], int i, int j);
		void write3Byte(int i);
		void writeDWord(int i);
		void writeDWord_v1(int i);
		void writeDWord_v2(int i);
		void writeDWordBigEndian(int i);
		void writeFrameSize(int i);
		void writeFrameSizeWord(int i);
		void writeQWord(unsigned __int64 l);
		void writeString(char* s);
		void writeWord(int i);
		void writeWordA(int i);
		void writeWordBigEndian(int i);
		void writeWordBigEndian_dup(int i);
		void writeWordBigEndianA(int i);

		void readBytes(char abyte0[], int i, int j);
		void readBytes_reverse(char abyte0[], int i, int j);
		void readBytes_reverseA(char abyte0[], int i, int j);
		unsigned int readDWord();
		int readDWord_v1();
		int readDWord_v2();
		unsigned __int64 readQWord();
		char readSignedByte();
		char readSignedByteA();
		char readSignedByteC();
		char readSignedByteS(); 
		int readSignedWord();
		int readSignedWordA();
		int readSignedWordBigEndian();
		int readSignedWordBigEndianA();
		int read3Byte();
		void readString(char* output);
		unsigned char readUnsignedByte();
		unsigned char readUnsignedByteA();
		unsigned char readUnsignedByteC();
		unsigned char readUnsignedByteS(); 
		int readUnsignedWord();
		int readUnsignedWordA();
		int readUnsignedWordBigEndian();
		int readUnsignedWordBigEndianA();

		char* buffer;
		int currentOffset; //offset of last position in buffer.

		void initBitAccess();
		void writeBits(int numBits, int value);
		void finishBitAccess();
	private:
		int bitPosition;
		int bitMaskOut[32];
		int frameStackPtr;
		int frameStack[frameStackSize];

};
#endif
