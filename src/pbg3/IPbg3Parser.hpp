#pragma once
#include "decomp.hpp"
#include <limits.h>
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
    u32 ReadInt();
    char ReadChar()
    {
        return this->ReadBits(CHAR_BIT);
    }
    BOOL ReadString(char *out, u32 maxSize);
    virtual BOOL ReadBit() = 0;
    virtual u32 ReadBits(u32 numBits) = 0;
    virtual i32 ReadByte() = 0;
    virtual BOOL SeekToOffset(DWORD fileOffset) = 0;
    virtual BOOL SeekToNextByte() = 0;
    virtual BOOL Read(void *data, DWORD bytesToRead) = 0;
    virtual BOOL GetLastWriteTime(LPFILETIME lastWriteTime) = 0;
    virtual ~IPbg3Parser()
    {
    }

  protected:
    u32 offset;
    u32 fileSize;
    u32 curByte;
    u8 bitMask;
    alignment_padding(0x3);
    u32 checksum;
};
} // namespace th06
