#include "SDLManager.h"
#include <iostream>

// Constructor
SDLManager::SDLManager() : window(nullptr), renderer(nullptr), texture(nullptr) {}

// Destructor
SDLManager::~SDLManager()
{
    Cleanup(); // Ensure proper cleanup when the object is destroyed
}

// Initialize SDL, create the window and renderer, and load the image
bool SDLManager::Init(const std::string& windowTitle, int width, int height, const std::string& imagePath)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (IMG_Init(IMG_INIT_TIF) == -1)
    {
        std::cerr << "IMG_Init Error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    // Load the image
    SDL_Surface* imageSurface = IMG_Load(imagePath.c_str());
    if (!imageSurface)
    {
        std::cerr << "IMG_Load Error: " << IMG_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    // Create a window
    window = SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, imageSurface->w, imageSurface->h, SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(imageSurface);
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    // Create a renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(imageSurface);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    // Create a texture from the image surface
    texture = SDL_CreateTextureFromSurface(renderer, imageSurface);
    SDL_FreeSurface(imageSurface);
    if (!texture)
    {
        std::cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return false;
    }
    SDL_RaiseWindow(window);

    return true;
}

// Handle window events such as resize
void SDLManager::HandleEvents(bool& viewImage)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            viewImage = false;
        }
        else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_x)
        {
            viewImage = false;  // Exit image view mode when 'X' is pressed
        }
    }
}

void SDLManager::HandleGeneralEvents(bool& running)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            running = false;
        }
        // Additional event handling logic can be added here
    }
}

void SDLManager::HandleImageViewEvents(bool& viewImage)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_x))
        {
            viewImage = false;  // Exit image view mode and go back to the menu
        }
    }
}

// Render the texture
void SDLManager::Render()
{
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

// Clean up SDL resources
void SDLManager::Cleanup()
{
    if (texture)
    {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }

    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    IMG_Quit();
    SDL_Quit();
}

