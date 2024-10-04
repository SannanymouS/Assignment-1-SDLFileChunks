#include "Asset.h"
#include "Level.h"
#include "FileChunk.h"
#include <fstream>
#include <iostream>
#include <cstring> // memcpy

Level::Level(size_t totalSize) : imageBuffer(nullptr), totalSize(totalSize), currentOffset(0)
{
    // Initialize chunk status to false (none loaded yet)
    chunkStatus.resize(7, false);  // Assuming 7 chunks
}

Level::~Level()
{
    // Ensure resources are cleaned up
    DeleteImageBuffer();
}

// Creates image buffer
void Level::CreateImageBuffer(size_t totalSize)
{
    if (imageBuffer != nullptr)
    {
        std::cerr << "Image buffer already exists!" << std::endl;
        return;
    }

    // Allocate memory for the image buffer
    imageBuffer = malloc(totalSize);
    if (imageBuffer == nullptr)
    {
        std::cerr << "Failed to allocate memory for image buffer!" << std::endl;
        return;
    }

    // Initialize the buffer to zeros
    memset(imageBuffer, 0, totalSize);
    this->totalSize = totalSize;
    
    // unit test : image buffer size autoscaling adjusting
    //std::cout << "Image buffer created with size: " << totalSize << " bytes." << std::endl;
}

// Deletes image buffer and resets state
void Level::DeleteImageBuffer()
{
    if (imageBuffer == nullptr)
    {
        std::cerr << "No image buffer to delete!" << std::endl;
        return;
    }

    // Deallocate memory and reset metadata
    free(imageBuffer);
    imageBuffer = nullptr;
    totalSize = 0;
    currentOffset = 0;

    // Reset chunk status
    chunkStatus.assign(chunkStatus.size(), false);

    std::cout << "Image buffer deleted." << std::endl;
}

// Assembles chunks into the image buffer
bool Level::AssembleChunks(const std::vector<std::string>& chunkFiles, StackAllocator& allocator, ObjectPool<FileChunk>& fileChunkPool, const std::string& outputImagePath, ObjectPool<Asset>& assetPool)
{
    if (imageBuffer == nullptr)
    {
        std::cerr << "Image buffer is not created!" << std::endl;
        return false;
    }

    // Iterate through chunk files then add to image buffer
    for (size_t i = 0; i < chunkFiles.size(); ++i)
    {
        if (!AddChunk(static_cast<int>(i), chunkFiles[i], allocator, fileChunkPool, assetPool, undoStack))  // Explicit cast to int))
        {
            std::cerr << "Failed to add chunk: " << i << std::endl;
            return false;
        }
    }

    // After all chunks are processed
    std::cout << "LEVEL" << std::endl;
    std::cout << "BASE RESOURCE" << std::endl;

    // Save the assembled image
    return SaveImage(outputImagePath);
}

bool Level::AddChunk(int chunkIndex, const std::string& chunkFile, StackAllocator& allocator, ObjectPool<FileChunk>& fileChunkPool, ObjectPool<Asset>& assetPool, std::stack<std::string>& undoStack)
{
    if (chunkIndex < 0 || chunkIndex >= chunkStatus.size())
    {
        std::cerr << "Invalid chunk index!" << std::endl;
        return false;
    }

    // If the chunk is loaded, skip
    if (chunkStatus[chunkIndex])
    {    // unit test - reusing object pools
        //std::cerr << "Chunk " << chunkIndex << " already added!" << std::endl;
        return true;
    }

    // Acquire a new FileChunk object from the pool before logging the asset allocation
    FileChunk* chunk = fileChunkPool.Acquire();

    // Acquire a new Asset object from the pool before logging the asset allocation
    Asset* asset = assetPool.Acquire();


    // Log the asset to UI
    std::cout << "Allocating asset " << chunkFile << std::endl;

    // Load the chunk and allocate memory
    std::ifstream inputChunk(chunkFile, std::ios::binary);
    if (!inputChunk)
    {
        std::cerr << "Failed to open chunk file: " << chunkFile << std::endl;
        return false;
    }

    inputChunk.seekg(0, std::ios::end);
    size_t chunkSize = inputChunk.tellg();
    inputChunk.seekg(0, std::ios::beg);

    void* chunkData = allocator.Allocate(chunkSize);
    if (!chunkData)
    {
        std::cerr << "Failed to allocate memory for chunk " << chunkIndex << std::endl;
        return false;
    }

    inputChunk.read(static_cast<char*>(chunkData), chunkSize);


    // Load data into the FileChunk object
    chunk->LoadData(chunkData, chunkSize);

    // Release the asset back to pool
    assetPool.Release(asset);

    // Copy the chunk data into the image buffer at the current offset
    memcpy(static_cast<char*>(imageBuffer) + currentOffset, chunkData, chunkSize);
    currentOffset += chunkSize;

    // Update chunk status
    chunkStatus[chunkIndex] = true;

    // Add the action to the undo stack for undo functionality
    undoStack.push("AddChunk " + std::to_string(chunkIndex));

    return true;
}

// Removes chunk from the image buffer (zeros out the memory)
void Level::RemoveChunk(int chunkIndex)
{
    if (chunkIndex < 0 || chunkIndex >= chunkStatus.size() || !chunkStatus[chunkIndex])
    {
        std::cerr << "Invalid or non-existent chunk to remove!" << std::endl;
        return;
    }

    // Get chunk start position and size
    void* chunkStart = GetChunkStart(chunkIndex);
    size_t chunkSize = GetChunkSize(chunkIndex);
    if (!chunkStart || chunkSize == 0)  // unit test
    {
        std::cerr << "Failed to retrieve chunk memory or size for chunk " << chunkIndex << "." << std::endl;
        return;
    }


    // Zero out the chunk memory
    memset(chunkStart, 0, chunkSize);

    // Update the chunk status
    chunkStatus[chunkIndex] = false;
    undoStack.push("RemoveChunk " + std::to_string(chunkIndex));
    std::cout << "Chunk " << chunkIndex << " removed." << std::endl;
    SaveImage("NewImage.tga");
}

// Gets starting address of a chunk
std::string Level::GetChunkFile(int chunkIndex)
{
    // Assuming you have a vector `chunkFiles` in your class that stores the file paths of chunks
    if (chunkIndex >= 0 && chunkIndex < chunkFiles.size())
    {
        return chunkFiles[chunkIndex];  // Returns the file corresponding to the chunk index
    }
    else
    {
        std::cerr << "Invalid chunk index!" << std::endl;
        return "";
    }
}

// Saves current image buffer to output file
bool Level::SaveImage(const std::string& outputImagePath)
{
    std::ofstream outputImage(outputImagePath, std::ios::binary);
    if (!outputImage)
    {
        std::cerr << "Failed to save image to: " << outputImagePath << std::endl;
        return false;
    }

    outputImage.write(static_cast<char*>(imageBuffer), totalSize);
    // unit test - save to image filepath
    // std::cout << "Image saved to " << outputImagePath << std::endl;
    return true;
}

// Calculate total size of chunk files
size_t Level::CalculateTotalChunkSize(const std::vector<std::string>& chunkFiles)
{
    size_t totalSize = 0;
    for (const auto& chunkFile : chunkFiles)
    {
        std::ifstream inputChunk(chunkFile, std::ios::binary | std::ios::ate);
        if (!inputChunk)
        {
            std::cerr << "Failed to open chunk file: " << chunkFile << std::endl;
            continue;
        }
        // Get the size of the file
        size_t chunkSize = inputChunk.tellg();
        totalSize += chunkSize;
    }
    return totalSize;
}

// Gets image buffer in main loop
void* Level::GetImageBuffer() const
{
    return imageBuffer;
}

bool Level::LoadLevel(const std::string& filename, StackAllocator& allocator, ObjectPool<FileChunk>& fileChunkPool, ObjectPool<Asset>& assetPool)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        std::cerr << "Failed to open file: " << filename << " for reading." << std::endl;
        return false;
    }

    // Clear current chunk data
    chunkPointers.clear();
    chunkStatus.assign(chunkStatus.size(), false);
    currentOffset = 0;

    // Read chunk data from the file
    while (!file.eof())
    {
        size_t chunkSize = 0;

        // Read the size of the chunk
        file.read(reinterpret_cast<char*>(&chunkSize), sizeof(chunkSize));
        if (file.eof())
            break;  // Stop if we've reached the end of the file

        // Allocate memory for the chunk data
        void* chunkData = allocator.Allocate(chunkSize);
        if (!chunkData)
        {
            std::cerr << "Failed to allocate memory for chunk!" << std::endl;
            return false;
        }

        // Read the chunk data into the allocated memory
        file.read(static_cast<char*>(chunkData), chunkSize);

        // Create a new FileChunk and load the data
        FileChunk* chunk = fileChunkPool.Acquire();
        Asset* asset = assetPool.Acquire();
        chunk->LoadData(chunkData, chunkSize);

        // Store the chunk in the chunk pointers array and mark it as loaded
        chunkPointers.push_back(chunk);  // Store the chunk pointer
        chunkStatus[chunkPointers.size() - 1] = true;  // Mark the chunk as loaded

        std::cout << "Chunk of size " << chunkSize << " loaded." << std::endl;
    }

    file.close();
    return true;
}

bool Level::SaveLevel(const std::string& fileName)
{
    std::ofstream outFile(fileName, std::ios::binary);
    if (!outFile)
    {
        std::cerr << "Failed to open file: " << fileName << " for writing." << std::endl;
        return false;
    }

    std::cout << "Starting to save level..." << std::endl;

    // Iterate over the chunkStatus array to find loaded chunks
    for (int chunkIndex = 0; chunkIndex < chunkStatus.size(); ++chunkIndex)
    {
        if (chunkStatus[chunkIndex])  // Check if the chunk is loaded
        {
            if (chunkIndex < chunkPointers.size())  // Ensure we are within bounds
            {
                FileChunk* chunk = chunkPointers[chunkIndex];  // Retrieve the chunk from memory
                if (!chunk)
                {
                    std::cerr << "Error: Chunk pointer is null for index " << chunkIndex << std::endl;
                    return false;
                }

                // Write the size of the chunk
                size_t chunkSize = chunk->GetSize();
                outFile.write(reinterpret_cast<const char*>(&chunkSize), sizeof(chunkSize));

                // Write the raw chunk data to the file
                outFile.write(static_cast<const char*>(chunk->GetData()), chunkSize);
                std::cout << "Chunk " << chunkIndex << " saved successfully." << std::endl;
            }
            else
            {
                std::cerr << "Error: chunkIndex " << chunkIndex << " out of bounds in chunkPointers!" << std::endl;
                return false;
            }
        }
    }

    outFile.close();
    std::cout << "Level saved successfully to " << fileName << std::endl;
    return true;
}

int Level::GetCurrentChunkIndex() const
{
    return currentChunkIndex;
}

// GetChunkStart: Returns the start position of a chunk in the image buffer
void* Level::GetChunkStart(int chunkIndex)
{
    if (chunkIndex < 0 || chunkIndex >= chunkPointers.size())
    {
        std::cerr << "Invalid chunk index!" << std::endl;
        return nullptr;
    }

    // Calculate the start position of the chunk
    size_t offset = 0;
    for (int i = 0; i < chunkIndex; ++i)
    {
        offset += GetChunkSize(i);
    }

    return static_cast<char*>(imageBuffer) + offset;
}

// GetChunkSize: Returns the size of a specific chunk
size_t Level::GetChunkSize(int chunkIndex)
{
    if (chunkIndex < 0 || chunkIndex >= chunkPointers.size())
    {
        std::cerr << "Invalid chunk index!" << std::endl;
        return 0;
    }

    return chunkPointers[chunkIndex]->GetSize();
}

// IsChunkLoaded: Checks if the chunk at the given index is loaded
bool Level::IsChunkLoaded(int chunkIndex) const
{
    if (chunkIndex < 0 || chunkIndex >= chunkStatus.size())
    {
        std::cerr << "Invalid chunk index!" << std::endl;
        return false;
    }
    return chunkStatus[chunkIndex];
}

// Add a chunk for testing purposes
void Level::AddChunkForTest(int chunkIndex, FileChunk* chunk)
{
    if (chunkIndex >= chunkPointers.size())
    {
        chunkPointers.resize(chunkIndex + 1, nullptr);
        chunkStatus.resize(chunkIndex + 1, false);
    }

    chunkPointers[chunkIndex] = chunk;
    chunkStatus[chunkIndex] = true;  // Mark the chunk as loaded
}

void Level::TestIsChunkLoaded()
{
    for (int i = 0; i < chunkStatus.size(); ++i)
    {
        std::cout << "Chunk " << i << " loaded status: " << (chunkStatus[i] ? "Loaded" : "Not Loaded") << std::endl;
    }
}

void Level::TestGetChunkStartAndSize()
{
    std::cout << "Testing GetChunkStart and GetChunkSize...\n";

    void* chunkStart = GetChunkStart(0);
    size_t chunkSize = GetChunkSize(0);

    if (chunkStart && chunkSize > 0)
    {
        std::cout << "Chunk 0 start at: " << chunkStart << ", size: " << chunkSize << "\n";
    }
    else
    {
        std::cout << "Failed to retrieve chunk start or size for chunk 0\n";
    }
}


