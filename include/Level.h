#ifndef LEVEL_H
#define LEVEL_H

#include "StackAllocator.h"
#include "ObjectPool.h"
#include "FileChunk.h"
#include <vector>
#include <string>
#include <stack>


class FileChunk {
public:
    size_t GetSize() const { return size; }
private:
    size_t size;
};

class Level
{
public:
    Level() : totalSize(0), currentOffset(0), imageBuffer(nullptr) {}
    Level(size_t totalSize);
    ~Level();

    std::stack<std::string> undoStack;

    bool SaveLevel(const std::string& fileName);
    bool LoadLevel(const std::string& fileName, StackAllocator& allocator, ObjectPool<FileChunk>& fileChunkPool, ObjectPool<Asset>& assetPool);  // for loading the chunks back

    // Creates the image buffer with the given total size
    void CreateImageBuffer(size_t totalSize);

    // Deletes the image buffer and resets the state
    void DeleteImageBuffer();

    // Assemble chunks from chunk files and write to output image
    bool AssembleChunks(const std::vector<std::string>& chunkFiles, StackAllocator& allocator, ObjectPool<FileChunk>& pool, const std::string& outputImagePath, ObjectPool<Asset>& assetPool );

    // Statically calculate total chunk size
    static size_t CalculateTotalChunkSize(const std::vector<std::string>& chunkFiles);

    // Adds a chunk to the image buffer
    bool AddChunk(int chunkIndex, const std::string& chunkFile, StackAllocator& allocator, ObjectPool<FileChunk>& fileChunkPool, ObjectPool<Asset>& assetPool, std::stack<std::string>& undoStack);
    
    // Removes a chunk from the image buffer
    void RemoveChunk(int chunkIndex);

    // Save the assembled image to a file
    bool SaveImage(const std::string& outputImagePath);

    // Gets image buffer in main loop
    void* GetImageBuffer() const;

    int GetCurrentChunkIndex() const;

    // Getters for chunk information
    void* GetChunkStart(int chunkIndex);
    size_t GetChunkSize(int chunkIndex);

    // Status to track if a chunk is loaded or removed
    bool IsChunkLoaded(int chunkIndex) const;


    // Unit test helpers
    void TestGetChunkStartAndSize();
    void TestIsChunkLoaded();

    // Helper to add a chunk for testing
    void AddChunkForTest(int chunkIndex, FileChunk* chunk);

    std::string GetChunkFile(int chunkIndex);


private:
    std::vector<FileChunk*> fileChunks;
    std::vector<FileChunk*> chunkPointers;
    void* imageBuffer;
    size_t totalSize;
    size_t currentOffset = 0;  // Current offset in image buffer      
    std::vector<bool> chunkStatus; // Tracks if chunks are loaded
    int currentChunkIndex;
    std::vector<std::string> chunkFiles;
};

#endif // LEVEL_H
