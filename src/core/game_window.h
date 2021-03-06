#ifndef KINGDOM_GAME_WINDOW_H
#define KINGDOM_GAME_WINDOW_H

#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"

class GameWindow {
  uint32_t window_width;
  uint32_t window_height;
  uint32_t clock;

  SDL_Window* sdl_window;
  SDL_GLContext gl_context;

public:
  GameWindow(const char* title, uint32_t width, uint32_t height);
  ~GameWindow();

  uint32_t Width() const;
  uint32_t Height() const;
  SDL_Window* Handle() const;
  SDL_GLContext GLContext() const;

  float Dt();
  void SwapBuffers();
};

#endif //KINGDOM_GAME_WINDOW_H
