#pragma once
#include "decomp.hpp"
#include <Windows.h>

namespace th06
{
class IPbg3Parser
{
  public:
    IPbg3Parser()
    {
        this->Reset();
    }
    void Reset();
    u32 ReadVarInt();
    u32 ReadMagic();
    BOOL ReadString(char *out, u32 maxSize);
    virtual BOOL ReadBit() = 0;
    virtual u32 ReadInt(u32 numBitsAsPowersOf2) = 0;
    virtual i32 ReadByte() = 0;
    virtual BOOL SeekToOffset(u32 fileOffset) = 0;
    virtual BOOL SeekToNextByte() = 0;
    virtual BOOL Read(u8 *data, u32 bytesToRead) = 0;
    virtual BOOL GetLastWriteTime(LPFILETIME lastWriteTime) = 0;
    virtual ~IPbg3Parser()
    {
    }

  protected:
    u32 offsetInFile;
    u32 fileSize;
    u32 curByte;
    u8 bitIdxInCurByte;
    u32 crc;
};
} // namespace th06
