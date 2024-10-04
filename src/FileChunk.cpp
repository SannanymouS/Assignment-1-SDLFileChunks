#include "FileChunk.h"


FileChunk::FileChunk() : data(nullptr), size(0) {}

void FileChunk::LoadData(void* chunkData, size_t chunkSize)
{
    data = chunkData;
    size = chunkSize;
}

void* FileChunk::GetData()
{
    return data;
}

size_t FileChunk::GetSize()
{
    return size;
}
