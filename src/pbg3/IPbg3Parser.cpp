#include "pbg3/IPbg3Parser.hpp"

namespace th06
{
void IPbg3Parser::Reset()
{
    this->bitMask = 128;
    this->offset = 0;
    this->fileSize = 0;
    this->curByte = 0;
    this->checksum = 0;
}

u32 IPbg3Parser::ReadVarInt()
{
    u32 res = 0;
    i32 varintHdr = 0;

    if (this->ReadBit())
    {
        varintHdr = 2;
    }
    if (this->ReadBit())
    {
        varintHdr |= 1;
    }

    u32 intLen = 0;
    switch (varintHdr)
    {
    case 0:
        intLen = 0x80;
        break;
    case 1:
        intLen = 0x8000;
        break;
    case 2:
        intLen = 0x800000;
        break;
    case 3:
        intLen = 0x80000000;
        break;
    }

    while (intLen != 0)
    {
        if (this->ReadBit())
        {
            res |= intLen;
        }
        intLen >>= 1;
    }

    return res;
}

u32 IPbg3Parser::ReadInt()
{
    u32 b0 = this->ReadBits(8);
    u32 b1 = b0 + (this->ReadBits(8) << 8);
    u32 b2 = b1 + (this->ReadBits(8) << 16);
    u32 b3 = b2 + (this->ReadBits(8) << 24);

    return b3;
}

BOOL IPbg3Parser::ReadString(char *out, u32 maxSize)
{
    if (out == NULL)
        return FALSE;

    for (u32 idx = 0; idx < maxSize; idx++)
    {
        out[idx] = this->ReadChar();
        if (out[idx] == '\0')
        {
            return TRUE;
        }
    }

    return FALSE;
}

u8 IPbg3Parser::ReadUnalignedByte()
{
    return this->ReadInt(8);
}
} // namespace th06
