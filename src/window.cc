#include "window.h"

#include <SDL2/SDL.h>
#include <cstdio>

// TODO: proper error handling
yb::Window::Window(const char* title, int width, int height)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::fputs("Unable to initialize SDL.", stderr);
    }

    window_ = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width,
        height,
        SDL_WINDOW_SHOWN
    );

    if (window_ == nullptr) {
        std::fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
    }

    surface_ = SDL_GetWindowSurface(window_);

    // Fill white for now
    SDL_FillRect(surface_, nullptr, SDL_MapRGB(surface_->format, 0xFF, 0xFF, 0xFF));
}

void yb::Window::draw()
{
    SDL_UpdateWindowSurface(window_);
}

yb::Window::~Window()
{
    SDL_FreeSurface(surface_);

    SDL_DestroyWindow(window_);

    SDL_Quit();
}