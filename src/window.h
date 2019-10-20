#pragma once

struct SDL_Window;
struct SDL_Surface;

namespace yb {

    class Window
    {
    public:
        Window(const char* title, int width, int height);
        ~Window();

        Window(Window&&) = default;
        Window& operator=(Window&&) = default;

        void draw();

    private:
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        SDL_Window* window_;
        SDL_Surface* surface_;
    };
}