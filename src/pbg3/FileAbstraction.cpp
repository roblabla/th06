#include "pbg3/FileAbstraction.hpp"

namespace th06
{
FileAbstraction::FileAbstraction()
{
    this->handle = INVALID_HANDLE_VALUE;
    this->access = 0;
}

FileAbstraction::~FileAbstraction()
{
    Close();
}

BOOL FileAbstraction::Open(const char *filename, char *mode)
{
    char *curMode;
    BOOL isAppendMode = FALSE;
    DWORD creationDisposition;

    Close();

    for (curMode = mode; *curMode != '\0'; curMode++)
    {
        if (*curMode == 'r')
        {
            access = GENERIC_READ;
            creationDisposition = OPEN_EXISTING;
            break;
        }
        if (*curMode == 'w')
        {
            DeleteFile(filename);
            access = GENERIC_WRITE;
            creationDisposition = OPEN_ALWAYS;
            break;
        }
        if (*curMode == 'a')
        {
            isAppendMode = TRUE;
            access = GENERIC_WRITE;
            creationDisposition = OPEN_ALWAYS;
            break;
        }
    }

    if (*curMode == '\0')
        return FALSE;

    handle = CreateFile(filename, this->access, FILE_SHARE_READ, NULL, creationDisposition,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (INVALID_HANDLE_VALUE == this->handle)
        return FALSE;

    if (isAppendMode)
        SetFilePointer(this->handle, 0, NULL, FILE_END);

    return TRUE;
}

void FileAbstraction::Close(void)
{
    if (INVALID_HANDLE_VALUE != handle)
    {
        CloseHandle(handle);
        handle = INVALID_HANDLE_VALUE;
        access = 0;
    }
}

BOOL FileAbstraction::Read(void *data, DWORD dataLen, DWORD *numBytesRead)
{
    if (GENERIC_READ != access)
        return FALSE;

    return ReadFile(handle, data, dataLen, numBytesRead, NULL);
}

BOOL FileAbstraction::Write(void *data, DWORD dataLen, DWORD *outWritten)
{
    if (GENERIC_WRITE != access)
        return FALSE;

    return WriteFile(handle, data, dataLen, outWritten, NULL);
}

int FileAbstraction::ReadByte()
{
    BYTE data;
    DWORD outBytesRead;

    if (FALSE == this->Read(&data, 1, &outBytesRead))
        return PBG_EOF;
    if (outBytesRead == 0)
        return PBG_EOF;
    else
        return data;
}

int FileAbstraction::WriteByte(int b)
{
    BYTE outByte;
    DWORD outBytesWritten;

    outByte = b;
    if (FALSE == this->Write(&outByte, 1, &outBytesWritten))
        return PBG_EOF;
    if (outBytesWritten == 0)
        return PBG_EOF;
    else
        return b;
}

BOOL FileAbstraction::Seek(DWORD amount, DWORD seekFrom)
{
    if (INVALID_HANDLE_VALUE == this->handle)
        return FALSE;

    SetFilePointer(this->handle, amount, NULL, seekFrom);
    return TRUE;
}

DWORD FileAbstraction::Tell(void)
{
    if (INVALID_HANDLE_VALUE == handle)
        return 0;

    return SetFilePointer(handle, 0, NULL, FILE_CURRENT);
}

DWORD FileAbstraction::GetSize(void)
{
    if (INVALID_HANDLE_VALUE == handle)
        return 0;

    return GetFileSize(handle, NULL);
}

BOOL FileAbstraction::WriteString(void *buffer)
{
    DWORD Length;
    DWORD temp;

    if (GENERIC_WRITE != access)
        return FALSE;

    Length = strlen((char *)buffer);
    return Write(buffer, Length, &temp);
}

LPVOID FileAbstraction::ReadWholeFile(DWORD maxSize)
{
    DWORD oldLocation, dataLen, outDataLen;
    LPVOID data;

    if (GENERIC_READ != this->access)
        return NULL;

    dataLen = this->GetSize();
    if (dataLen > maxSize)
        return NULL;

    data = LocalAlloc(LPTR, dataLen);
    if (NULL == data)
        return NULL;

    oldLocation = Tell();

    // Pretty sure the plan here was to seek to 0, but woops the code
    // is buggy. And yes, this case leaks the data. Amazing, I know.
    if (FALSE == Seek(oldLocation, FILE_BEGIN))
        return NULL;

    if (FALSE == Read(data, dataLen, &outDataLen))
    {
        LocalFree(data);
        return NULL;
    }

    Seek(oldLocation, FILE_BEGIN);

    return data;
}

} // namespace th06
