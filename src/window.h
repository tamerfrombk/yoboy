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

        void update();

        bool isQuit() const noexcept;

    private:
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        SDL_Window* window_;
        SDL_Surface* surface_;

        bool isQuit_;
    };
}