//
// Created by Trevor Fisher on 6/4/2021.
//

#include <cassert>
#include "game_window.h"
#include "glad/glad.h"

const int OPENGL_MAJOR_VERSION = 4;
const int OPENGL_MINOR_VERSION = 0;

GameWindow::GameWindow(const char *title, uint32_t width, uint32_t height) {
  SDL_SetMainReady();

  // Initialize SDL and create a window
  SDL_Init(SDL_INIT_VIDEO);

  // Set SDL OpenGL attributes (core profile, opengl v4.0)
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_MAJOR_VERSION);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_MINOR_VERSION);

  // Set multisampling
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);

  // SDL_GL_SetSwapInterval(0); // vsync - needs testing

  // TODO: Handle high dpi scaling
  float dpi = 96.f; // standard dpi
  /*if (SDL_GetDisplayDPI(0, NULL, &dpi, NULL) != 0 || dpi == 0.f) {
    // error => Failed to get DPI info, falling back to default
    dpi = 96.f;
  }*/

  window_width = uint32_t(width * dpi / 96.f);
  window_height = uint32_t(height * dpi / 96.f);

  // TODO: Implement logging

  // Window creation
  sdl_window = SDL_CreateWindow(
    title,
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    int(window_width), int(window_height),
    SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI
  );
  assert(sdl_window != nullptr);

  // OpenGL context creation
  gl_context = SDL_GL_CreateContext(sdl_window);
  clock = SDL_GetTicks();

  int gladLoadSuccess = gladLoadGLLoader(SDL_GL_GetProcAddress);
  assert(gladLoadSuccess);
}
GameWindow::~GameWindow() {
  SDL_GL_DeleteContext(gl_context);
  SDL_Quit();
}

uint32_t GameWindow::width() const {
  return window_width;
}

uint32_t GameWindow::height() const {
  return window_height;
}

/// Returns the time (in seconds) since the previous calling of dt()
float GameWindow::dt() {
  // Calculate the delta time from the previous call to dt()
  uint32_t ticks = SDL_GetTicks();
  float dt = float(ticks - clock) * 0.001f;
  clock = ticks;

  return dt;
}

void GameWindow::swap_buffers() {
  SDL_GL_SwapWindow(sdl_window);
}
