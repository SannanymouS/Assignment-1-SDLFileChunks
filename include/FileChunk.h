#ifndef FILECHUNK_H
#define FILECHUNK_H

#include <cstddef>

class FileChunk
{
public:
    FileChunk();  // Declare constructor

    void LoadData(void* chunkData, size_t chunkSize);

    void* GetData();

    size_t GetSize();

private:
    void* data;
    size_t size;
};

#endif // FILECHUNK_H
