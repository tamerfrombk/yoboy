#include "window.h"

#include <SDL2/SDL.h>
#include <cstdio>

// TODO: proper error handling
yb::Window::Window(const char* title, int width, int height)
    : isQuit_(false)
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

void::yb::Window::update()
{
    SDL_Event e;
    if (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            isQuit_ = true;
        }
    }
}

bool yb::Window::isQuit() const noexcept
{
    return isQuit_;
}

yb::Window::~Window()
{
    SDL_FreeSurface(surface_);

    SDL_DestroyWindow(window_);

    SDL_Quit();
}