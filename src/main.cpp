#include <iostream>
#include "glad/glad.h"
#include "core/game_window.h"
#include "graphics/sprite_batch.h"
#include "graphics/texture_2d.h"
#include "spdlog/spdlog.h"

int main() {
  spdlog::info("Initializing game...");
  const int viewport_width = 1280;
  const int viewport_height = 720;

  auto window = GameWindow("Kingdom", viewport_width, viewport_height);
  auto spriteBatch = SpriteBatch(viewport_width, viewport_height);
  auto texture = Texture2D("content/textures/placeholder.png");

  // fixed step logic
  float timer = window.Dt();
  const float fixed_step = 0.01667f; // 1/60th of a second

  SDL_Event sdlEvent;
  while (true) {
    if (SDL_PollEvent(&sdlEvent)) {
      if (sdlEvent.type == SDL_QUIT) break;
    }

    // Update logic
    timer += window.Dt();
    if (timer >= fixed_step) {
      timer -= fixed_step;
    }

    // Rendering logic
    glClear(GL_COLOR_BUFFER_BIT);
    auto pos = glm::vec2(640.f - texture.Width() / 2.f, 360 - texture.Height() / 2.f);
    spriteBatch.Draw(texture, pos);
    window.SwapBuffers();
  }

  spdlog::info("Program execution finished.");
  return 0;
}
