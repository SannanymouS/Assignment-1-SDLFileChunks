#ifndef SDLMANAGER_H
#define SDLMANAGER_H

#include <SDL.h>
#include <SDL_image.h>
#include <string>

class SDLManager
{
public:
    SDLManager();
    ~SDLManager();

    // Initialize SDL and create the window and renderer
    bool Init(const std::string& windowTitle, int width, int height, const std::string& imagePath);

    // Handle events such as window resizing
    void HandleEvents(bool& running);

    // Declare the HandleGeneralEvents function
    void HandleGeneralEvents(bool& running);

    // If you have the HandleImageViewEvents function as well, declare it here:
    void HandleImageViewEvents(bool& viewImage);

    // Render the current texture
    void Render();

    // Clean up SDL resources
    void Cleanup();

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
};

#endif // SDLMANAGER_H