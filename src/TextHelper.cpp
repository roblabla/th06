#include "stdafx.h"

#include "TextHelper.hpp"

namespace th06
{
struct FormatInfo
{
    D3DFORMAT format;
    i32 bitCount;
    u32 alphaMask;
    u32 redMask;
    u32 greenMask;
    u32 blueMask;
};

class TextHelper
{
  public:
    TextHelper();
    ~TextHelper();
    bool AllocateBufferWithFallback(i32 width, i32 height, D3DFORMAT format);
    bool TryAllocateBuffer(i32 width, i32 height, D3DFORMAT format);
    FormatInfo *GetFormatInfo(D3DFORMAT format);
    bool ReleaseBuffer();
    bool InvertAlpha(i32 x, i32 y, i32 spriteWidth, i32 fontHeight);
    bool CopyTextToSurface(LPDIRECT3DSURFACE8 outSurface);

    bool IsAllocated()
    {
        return this->gdiObj2 != NULL;
    }

    D3DFORMAT GetFormat()
    {
        return this->format;
    }

    i32 GetWidth()
    {
        return this->width;
    }

    i32 GetHeight()
    {
        return this->height;
    }

    u32 GetImageWidthInBytes()
    {
        return this->imageWidthInBytes;
    }

    HDC GetHDC()
    {
        return this->hdc;
    }

    u8 *GetBuffer()
    {
        return (u8 *)this->buffer;
    }

    D3DFORMAT format;
    i32 width;
    i32 height;
    u32 imageSizeInBytes;
    i32 imageWidthInBytes;
    HDC hdc;
    HGDIOBJ gdiObj;
    HGDIOBJ gdiObj2;
    void *buffer;
};
ZUN_ASSERT_TYPE(TextHelper, 0x24, 4);

FormatInfo g_FormatInfoArray[] = {
    {D3DFMT_X8R8G8B8, 32, 0x00000000, 0x00FF0000, 0x0000FF00, 0x000000FF},
    {D3DFMT_A8R8G8B8, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF},
    {D3DFMT_X1R5G5B5, 16, 0x00000000, 0x00007C00, 0x000003E0, 0x0000001F},
    {D3DFMT_R5G6B5, 16, 0x00000000, 0x0000F800, 0x000007E0, 0x0000001F},
    {D3DFMT_A1R5G5B5, 16, 0x00008000, 0x00007C00, 0x000003E0, 0x0000001F},
    {D3DFMT_A4R4G4B4, 16, 0x0000F000, 0x00000F00, 0x000000F0, 0x0000000F},
    {(D3DFORMAT)-1, 0, 0, 0, 0, 0},
};

TextHelper::TextHelper()
{
    this->format = (D3DFORMAT)-1;
    this->width = 0;
    this->height = 0;
    this->hdc = NULL;
    this->gdiObj2 = NULL;
    this->gdiObj = NULL;
    this->buffer = NULL;
}

TextHelper::~TextHelper()
{
    this->ReleaseBuffer();
}

bool TextHelper::ReleaseBuffer()
{
    if (this->hdc)
    {
        SelectObject(this->hdc, this->gdiObj);
        DeleteDC(this->hdc);
        DeleteObject(this->gdiObj2);
        this->format = (D3DFORMAT)-1;
        this->width = 0;
        this->height = 0;
        this->hdc = NULL;
        this->gdiObj2 = NULL;
        this->gdiObj = NULL;
        this->buffer = NULL;
        return true;
    }
    else
    {
        return false;
    }
}

bool TextHelper::AllocateBufferWithFallback(i32 width, i32 height, D3DFORMAT format)
{
    if (this->TryAllocateBuffer(width, height, format))
    {
        return true;
    }

    if (format == D3DFMT_A1R5G5B5 || format == D3DFMT_A4R4G4B4)
    {
        return this->TryAllocateBuffer(width, height, D3DFMT_A8R8G8B8);
    }
    if (format == D3DFMT_R5G6B5)
    {
        return this->TryAllocateBuffer(width, height, D3DFMT_X8R8G8B8);
    }
    return false;
}

void strlen_dummy(const char *a)
{
    strlen(a);
}

#pragma var_order(imageWidthInBytes, deviceContext, originalBitmapObj, bitmapInfo, formatInfo, bitmapObj, bitmapData)
bool TextHelper::TryAllocateBuffer(i32 width, i32 height, D3DFORMAT format)
{
    HGDIOBJ originalBitmapObj;
    void *bitmapData;
    HBITMAP bitmapObj;
    FormatInfo *formatInfo;
    BITMAPV4HEADER bitmapInfo;
    HDC deviceContext;
    i32 imageWidthInBytes;

    this->ReleaseBuffer();
    memset(&bitmapInfo, 0, sizeof(BITMAPV4HEADER));
    formatInfo = this->GetFormatInfo(format);
    if (formatInfo == NULL)
    {
        return false;
    }
    imageWidthInBytes = ((((width * formatInfo->bitCount) / CHAR_BIT) + 3) / 4) * 4;
    bitmapInfo.bV4Size = sizeof(BITMAPV4HEADER);
    bitmapInfo.bV4Width = width;
    bitmapInfo.bV4Height = -(height + 1);
    bitmapInfo.bV4Planes = 1;
    bitmapInfo.bV4BitCount = formatInfo->bitCount;
    bitmapInfo.bV4SizeImage = height * imageWidthInBytes;
    if (format != D3DFMT_X1R5G5B5 && format != D3DFMT_X8R8G8B8)
    {
        bitmapInfo.bV4V4Compression = 3;
        bitmapInfo.bV4RedMask = formatInfo->redMask;
        bitmapInfo.bV4GreenMask = formatInfo->greenMask;
        bitmapInfo.bV4BlueMask = formatInfo->blueMask;
        bitmapInfo.bV4AlphaMask = formatInfo->alphaMask;
    }
    bitmapObj = CreateDIBSection(NULL, (BITMAPINFO *)&bitmapInfo, 0, &bitmapData, NULL, 0);
    if (bitmapObj == NULL)
    {
        return false;
    }
    memset(bitmapData, 0, bitmapInfo.bV4SizeImage);
    deviceContext = CreateCompatibleDC(NULL);
    originalBitmapObj = SelectObject(deviceContext, bitmapObj);
    this->hdc = deviceContext;
    this->gdiObj2 = bitmapObj;
    this->buffer = bitmapData;
    this->imageSizeInBytes = bitmapInfo.bV4SizeImage;
    this->gdiObj = originalBitmapObj;
    this->width = width;
    this->height = height;
    this->format = format;
    this->imageWidthInBytes = imageWidthInBytes;
    return true;
}

FormatInfo *TextHelper::GetFormatInfo(D3DFORMAT format)
{
    i32 i;

    for (i = 0; g_FormatInfoArray[i].format != -1 && g_FormatInfoArray[i].format != format; i++)
    {
    }
    if (format == -1)
    {
        return NULL;
    }
    return &g_FormatInfoArray[i];
}

struct A1R5G5B5
{
    u16 blue : 5;
    u16 green : 5;
    u16 red : 5;
    u16 alpha : 1;
};

#pragma var_order(bufferRegion, idx, doubleArea)
bool TextHelper::InvertAlpha(i32 x, i32 y, i32 spriteWidth, i32 fontHeight)
{
    i32 doubleArea;
    u8 *bufferRegion;
    i32 idx;

    doubleArea = spriteWidth * fontHeight * 2;
    bufferRegion = &this->GetBuffer()[y * spriteWidth * 2];
    switch (this->format)
    {
    case D3DFMT_A8R8G8B8:
        for (idx = 3; idx < doubleArea; idx += 4)
        {
            bufferRegion[idx] = bufferRegion[idx] ^ 0xff;
        }
        break;
    case D3DFMT_A1R5G5B5: {
        A1R5G5B5 *bufferCursor;
        for (bufferCursor = (A1R5G5B5 *)bufferRegion, idx = 0; idx < doubleArea; idx += 2, bufferCursor += 1)
        {
            bufferCursor->alpha ^= 1;
            if (bufferCursor->alpha)
            {
                bufferCursor->red = bufferCursor->red - bufferCursor->red * idx / doubleArea / 2;
                bufferCursor->green = bufferCursor->green - bufferCursor->green * idx / doubleArea / 2;
                bufferCursor->blue = bufferCursor->blue - bufferCursor->blue * idx / doubleArea / 4;
            }
            else
            {
                bufferCursor->red = 31 - idx * 31 / doubleArea / 2;
                bufferCursor->green = 31 - idx * 31 / doubleArea / 2;
                bufferCursor->blue = 31 - idx * 31 / doubleArea / 4;
            }
        }
        break;
    }
    case D3DFMT_A4R4G4B4:
        for (idx = 1; idx < doubleArea; idx += 2)
        {
            bufferRegion[idx] = bufferRegion[idx] ^ 0xf0;
        }
        break;
    default:
        return false;
    }
    return true;
}

#pragma var_order(dstBuf, dstWidthBytes, rectToLock, curHeight, srcWidthBytes, outSurfaceDesc, srcBuf, lockedRect)
bool TextHelper::CopyTextToSurface(LPDIRECT3DSURFACE8 outSurface)
{
    D3DLOCKED_RECT lockedRect;
    u8 *srcBuf;
    D3DSURFACE_DESC outSurfaceDesc;
    size_t srcWidthBytes;
    i32 curHeight;
    RECT rectToLock;
    int dstWidthBytes;
    u8 *dstBuf;

    if (!this->IsAllocated())
    {
        return false;
    }
    outSurface->GetDesc(&outSurfaceDesc);
    rectToLock.left = 0;
    rectToLock.top = 0;
    rectToLock.right = this->GetWidth();
    rectToLock.bottom = this->GetHeight();
    if (outSurface->LockRect(&lockedRect, &rectToLock, 0))
    {
        return false;
    }
    dstWidthBytes = lockedRect.Pitch;
    srcWidthBytes = this->GetImageWidthInBytes();
    srcBuf = this->GetBuffer();
    dstBuf = (u8 *)lockedRect.pBits;
    if (outSurfaceDesc.Format == this->GetFormat())
    {
        for (curHeight = 0; curHeight < this->GetHeight(); curHeight++)
        {
            memcpy(dstBuf, srcBuf, srcWidthBytes);
            srcBuf += srcWidthBytes;
            dstBuf += dstWidthBytes;
        }
    }
    outSurface->UnlockRect();
    return true;
}

void memmove_dummy(void *a, void *b, size_t c)
{
    memmove(a, b, c);
}

#define TEXT_BUFFER_HEIGHT 64
void TextHelper_CreateTextBuffer()
{
    g_Supervisor.d3dDevice->CreateImageSurface(GAME_WINDOW_WIDTH, TEXT_BUFFER_HEIGHT, D3DFMT_A1R5G5B5,
                                               &g_TextBufferSurface);
}

void TextHelper_ReleaseTextBuffer()
{
    SAFE_RELEASE(g_TextBufferSurface);
}

// Stand-in for a TextHelper function that /OPT:REF stripped from every build. Its
// references are still visible: 0.13a, 1.00 and 1.02h all pull DrawTextA right
// after Global's USER32 imports (only D3DX's own text code calls it). th10's
// TextHelper still has a DrawText-based renderer, so it was likely an early version.
void Fake_SetBkMode_SetTextColor_DrawTextA_CxxThrowException()
{
    SetBkMode(NULL, 0);
    SetTextColor(NULL, 0);
    DrawText(NULL, NULL, 0, NULL, 0);
    // NOTE: Using throw instead includes TypeInfo/RTTI
    // objects, which breaks library code order.
    _CxxThrowException(NULL, NULL);
}

#pragma var_order(hdc, font, textSurfaceDesc, h, textHelper, hdc, srcRect, destRect, destSurface)
void TextHelper_RenderTextToTexture(i32 xPos, i32 yPos, i32 spriteWidth, i32 spriteHeight, i32 fontHeight,
                                    i32 fontWidth, ZunColor textColor, ZunColor shadowColor, const char *string,
                                    LPDIRECT3DTEXTURE8 outTexture)
{
    HGDIOBJ h;
    RECT destRect;
    RECT srcRect;
    D3DSURFACE_DESC textSurfaceDesc;
    HFONT font;
    HDC hdc;

    font = CreateFont(fontHeight * 2, 0, 0, 0, FW_BOLD, false, false, false, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
                      CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_ROMAN | FIXED_PITCH, TH_FONT_NAME);
    TextHelper textHelper;
    g_TextBufferSurface->GetDesc(&textSurfaceDesc);
    textHelper.AllocateBufferWithFallback(textSurfaceDesc.Width, textSurfaceDesc.Height, textSurfaceDesc.Format);
    hdc = textHelper.GetHDC();
    h = SelectObject(hdc, font);
    textHelper.InvertAlpha(0, 0, spriteWidth * 2, fontHeight * 2 + 6);

    SetBkMode(hdc, TRANSPARENT);

    if (shadowColor != COLOR_WHITE)
    {
        // Render shadow.
        SetTextColor(hdc, shadowColor);
        TextOut(hdc, xPos * 2 + 3, 2, string, strlen(string));
    }
    // Render main text.
    SetTextColor(hdc, textColor);
    TextOut(hdc, xPos * 2, 0, string, strlen(string));

    SelectObject(hdc, h);
    textHelper.InvertAlpha(0, 0, spriteWidth * 2, fontHeight * 2 + 6);
    textHelper.CopyTextToSurface(g_TextBufferSurface);
    SelectObject(hdc, h);
    DeleteObject(font);
    destRect.left = 0;
    destRect.top = yPos;
    destRect.right = spriteWidth;
    destRect.bottom = yPos + 16;
    srcRect.left = 0;
    srcRect.top = 0;
    srcRect.right = spriteWidth * 2 - 2;
    srcRect.bottom = fontHeight * 2 - 2;
    {
        LPDIRECT3DSURFACE8 destSurface;
        outTexture->GetSurfaceLevel(0, &destSurface);
        D3DXLoadSurfaceFromSurface(destSurface, NULL, &destRect, g_TextBufferSurface, NULL, &srcRect, 4, 0);
        SAFE_RELEASE(destSurface);
    }
}
} // namespace th06
