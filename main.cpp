#include "Asset.h"
#include "Level.h"
#include "SDLManager.h"
#include "StackAllocator.h"
#include "ObjectPool.h"
#include <iostream>
#include <vector>
#include <stack>

std::stack<std::string> undoStack;
std::stack<std::string> redoStack;

void DisplayMenu(Level& level);
void HandleMenuAction(char choice, Level& level, bool& running, bool& viewImage, SDLManager& sdlManager, StackAllocator& allocator, ObjectPool<FileChunk>& fileChunkPool, ObjectPool<Asset>& assetPool, const std::vector<std::string>& chunkFiles);
void UndoAction(std::stack<std::string>& undoStack, std::stack<std::string>& redoStack, Level& level, StackAllocator& allocator, ObjectPool<FileChunk>& fileChunkPool, ObjectPool<Asset>& assetPool);
void RedoAction(std::stack<std::string>& undoStack, std::stack<std::string>& redoStack, Level& level, StackAllocator& allocator, ObjectPool<FileChunk>& fileChunkPool, ObjectPool<Asset>& assetPool);


int main(int argc, char* argv[])
{
    std::vector<std::string> chunkFiles = {
        "assets/chunk0.bin", "assets/chunk1.bin", "assets/chunk2.bin",
        "assets/chunk3.bin", "assets/chunk4.bin", "assets/chunk5.bin", "assets/chunk6.bin"
    };

    // Create output image file path & calculate the total chunk size
    const std::string outputImagePath = "NewImage.tga";
    size_t totalChunkSize = Level::CalculateTotalChunkSize(chunkFiles);
    Level level(totalChunkSize);

    int currentChunkIndex = 0;  // Initialize it to 0 or based on your logic

    // Create a stack allocator, object pool, level instance and image buffer
    StackAllocator allocator(totalChunkSize * 2);
    ObjectPool<FileChunk> fileChunkPool(7);
    ObjectPool<Asset> assetPool(7);

    // Initialize the Level
   // Level level;

    // Simulate adding a chunk for testing
    FileChunk chunk1;
    level.AddChunkForTest(0, &chunk1);  // Adding a chunk at index 0

    // Run the tests
    level.TestIsChunkLoaded();
    level.TestGetChunkStartAndSize();

    
    //Level level(totalChunkSize);
    //level.CreateImageBuffer(totalChunkSize);

    //TestIsChunkLoaded(level);
    //TestGetChunkStartAndSize(level);

    // **Add chunks to the level** using the loop
    for (int i = 0; i < 7; ++i)
    {   // unit test - chunk execution in UI
        //std::cout << "Processing chunk " << i << std::endl;
        level.AddChunk(i, chunkFiles[i], allocator, fileChunkPool, assetPool, undoStack);  // Pass assetPool as the fifth argument
    }

    // After initial chunk loading, disable initial run behavior in pools
    //fileChunkPool.DisableInitialRun();
    //assetPool.DisableInitialRun();


    if (!level.AssembleChunks(chunkFiles, allocator, fileChunkPool, "NewImage.tga", assetPool))
    {
        std::cerr << "Failed to assemble chunks." << std::endl;
        return -1;
    }

    // Initialize SDL Manager
    SDLManager sdlManager;

    bool running = true;
    bool viewImage = false;
    while (running)
    {
        if (!viewImage)  // Show menu when not viewing the image
        {
            DisplayMenu(level);

            char choice;
            std::cin >> choice;

            HandleMenuAction(choice, level, running, viewImage, sdlManager, allocator, fileChunkPool, assetPool, chunkFiles);
        }
        else
        {
            sdlManager.HandleEvents(viewImage);

            if (viewImage)
            {
                sdlManager.Render();
            }
            else
            {
                sdlManager.Cleanup();
                std::cout << "Returning to the menu...\n";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // Ignore leftover input
            }
        }
    }
    sdlManager.Cleanup();
    std::cout << "Exiting program...\n";
    return 0;
}

void DisplayMenu(Level& level)
{
    std::cout << "\n";
    std::cout << "[Q]uit   [S]ave   [L]oad level   [Z]Undo   [Y]Redo\n";
    std::cout << "[C]reate image buffer   [D]elete image buffer\n";
    std::cout << "[A]dd Chunk  [R]emove chunk   [V]iew Image(X to exit image)\n";
    std::cout << "Index (" << level.GetCurrentChunkIndex() << ")   ";
    std::cout << "   Undo count (" << undoStack.size() << ")   ";
    std::cout << "   Redo count (" << redoStack.size() << ")\n";
    std::cout << "Input: ";
}

void HandleMenuAction(char choice, Level& level, bool& running, bool& viewImage, SDLManager& sdlManager, StackAllocator& allocator, ObjectPool<FileChunk>& fileChunkPool, ObjectPool<Asset>& assetPool, const std::vector<std::string>& chunkFiles)
{
    switch (toupper(choice))
    {
    case 'Q':
        running = false;
        break;
    case 'S':
        if (level.SaveLevel("level.bin"))
        {
            std::cout << "Level saved to level.bin\n";
        }
        else
        {
            std::cerr << "Failed to save the level!\n";
        }
        break;
    case 'L':
    {
        std::cout << "Loading level...\n";
        size_t totalChunkSize = Level::CalculateTotalChunkSize(chunkFiles);  // Scoped inside the case
        if (level.LoadLevel("level.bin", allocator, fileChunkPool, assetPool))
        {
            std::cout << "Level loaded from level.bin\n";
        }
        else
        {
            std::cerr << "Failed to load the level!\n";
        }
        break;
    }
    case 'Z':
        UndoAction(undoStack, redoStack, level, allocator, fileChunkPool, assetPool);
        break;
    case 'Y':
        RedoAction(undoStack, redoStack, level, allocator, fileChunkPool, assetPool);
        break;
    case 'C':
    {
        std::cout << "Creating image buffer...\n";
        size_t totalChunkSize = Level::CalculateTotalChunkSize(chunkFiles);  // Properly scoped
        level.CreateImageBuffer(totalChunkSize);
        break;
    }
    case 'D':
        std::cout << "Deleting image buffer...\n";
        level.DeleteImageBuffer();
        break;
    case 'A':
    {
        int chunkIndex;
        std::cout << "Enter chunk index to add (0-6): ";
        std::cin >> chunkIndex;
        if (chunkIndex >= 0 && chunkIndex < 7)
        {
            level.AddChunk(chunkIndex, chunkFiles[chunkIndex], allocator, fileChunkPool, assetPool, undoStack);
            std::cout << "Adding chunk...\n";
        }
        else
        {
            std::cerr << "Invalid chunk index." << std::endl;
        }
        break;
    }
    case 'R':  // Remove chunk
    {
        int chunkIndex;
        std::cout << "Enter chunk index to remove (0-6): ";
        std::cin >> chunkIndex;
        if (chunkIndex >= 0 && chunkIndex < 7)
        {
            level.RemoveChunk(chunkIndex);
        }
        else
        {
            std::cerr << "Invalid chunk index. Enter number within range!" << std::endl;
        }
        break;
    }
    case 'V':  // View image
    {
        viewImage = true;
        if (!sdlManager.Init("SDLFileChunks", 800, 600, "NewImage.tga"))
        {
            std::cerr << "Failed to initialize SDL Manager" << std::endl;
            running = false;
        }
        break;
    }
    default:
        std::cerr << "Unknown option selected!" << std::endl;
        break;
    }
}

void UndoAction(std::stack<std::string>& undoStack, std::stack<std::string>& redoStack, Level& level, StackAllocator& allocator, ObjectPool<FileChunk>& fileChunkPool, ObjectPool<Asset>& assetPool)
{
    if (undoStack.empty()) {
        std::cerr << "No actions to undo." << std::endl;
        return;
    }

    std::string lastAction = undoStack.top();
    undoStack.pop();

    // Check if the last action was adding or removing a chunk
    if (lastAction.find("AddChunk") != std::string::npos) {
        int chunkIndex = std::stoi(lastAction.substr(lastAction.find(" ") + 1));

        // Undo the add by removing the chunk
        if (level.IsChunkLoaded(chunkIndex)) {
            level.RemoveChunk(chunkIndex);
            std::cout << "Undoing AddChunk action for chunk: " << chunkIndex << std::endl;
        }
    }
    else if (lastAction.find("RemoveChunk") != std::string::npos) {
        int chunkIndex = std::stoi(lastAction.substr(lastAction.find(" ") + 1));

        // Undo the removal by adding the chunk back
        std::cout << "Undoing RemoveChunk action for chunk: " << chunkIndex << std::endl;

        // Fetch the chunk file
        std::string chunkFile = level.GetChunkFile(chunkIndex);

        // Add the chunk back
        level.AddChunk(chunkIndex, chunkFile, allocator, fileChunkPool, assetPool, undoStack);

        std::cout << "Redoing AddChunk action for chunk: " << chunkIndex << std::endl;
    }

    redoStack.push(lastAction); // Add the undone action to the redo stack
}

void RedoAction(std::stack<std::string>& undoStack, std::stack<std::string>& redoStack, Level& level, StackAllocator& allocator, ObjectPool<FileChunk>& fileChunkPool, ObjectPool<Asset>& assetPool)
{
    if (redoStack.empty()) {
        std::cerr << "No actions to redo." << std::endl;
        return;
    }

    std::string lastRedo = redoStack.top();
    redoStack.pop();

    // Check if the last undone action was adding or removing a chunk
    if (lastRedo.find("AddChunk") != std::string::npos) {
        int chunkIndex = std::stoi(lastRedo.substr(lastRedo.find(" ") + 1));

        // Fetch the chunk file for the chunk being re-added
        std::string chunkFile = level.GetChunkFile(chunkIndex);

        // Redo the add chunk action
        level.AddChunk(chunkIndex, chunkFile, allocator, fileChunkPool, assetPool, undoStack);
        std::cout << "Redoing AddChunk action for chunk: " << chunkIndex << std::endl;
    }
    else if (lastRedo.find("RemoveChunk") != std::string::npos) {
        int chunkIndex = std::stoi(lastRedo.substr(lastRedo.find(" ") + 1));

        // Redo the remove chunk action
        level.RemoveChunk(chunkIndex);
        std::cout << "Redoing RemoveChunk action for chunk: " << chunkIndex << std::endl;
    }

    // Push the redone action back onto the undo stack
    undoStack.push(lastRedo);
}
