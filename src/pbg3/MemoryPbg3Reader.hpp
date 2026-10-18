#pragma once
#include "decomp.hpp"
#include "pbg3/IPbg3Reader.hpp"

namespace th06
{
class MemoryPbg3Reader : public IPbg3Reader
{
  public:
    MemoryPbg3Reader();
    virtual ~MemoryPbg3Reader();
    BOOL Open(u8 *data, u32 length, const FILETIME *time);
    void Close();
    virtual BOOL ReadBit();
    virtual u32 ReadBits(u32 numBits);
    virtual i32 ReadByte();
    virtual BOOL SeekToOffset(DWORD fileOffset);
    virtual BOOL SeekToNextByte();
    virtual BOOL Read(void *out, DWORD length);
    virtual BOOL GetLastWriteTime(LPFILETIME time);

  private:
    FILETIME lastWriteTime;
    u8 *data;
};
ZUN_ASSERT_SIZE(MemoryPbg3Reader, 0x24);
} // namespace th06
