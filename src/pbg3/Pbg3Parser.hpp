#pragma once
#include "decomp.hpp"
#include "pbg3/FileAbstraction.hpp"
#include "pbg3/IPbg3Parser.hpp"

namespace th06
{
class Pbg3Parser : public IPbg3Parser, public FileAbstraction
{
  public:
    Pbg3Parser();
    BOOL OpenArchive(const char *path);
    virtual void Close();
    virtual BOOL ReadBit();
    virtual u32 ReadBits(u32 numBits);
    virtual i32 ReadByte();
    virtual BOOL SeekToOffset(DWORD fileOffset);
    virtual BOOL SeekToNextByte();
    virtual BOOL Read(void *data, DWORD bytesToRead);
    virtual BOOL GetLastWriteTime(LPFILETIME lastWriteTime);

    ~Pbg3Parser();

    BOOL HasNonNullHandle()
    {
        return this->handle != NULL;
    }

    BOOL HasValidHandle()
    {
        return this->handle != INVALID_HANDLE_VALUE;
    }
};
ZUN_ASSERT_SIZE(Pbg3Parser, 0x24);
} // namespace th06
