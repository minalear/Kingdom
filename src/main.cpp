#include <iostream>
#include "glad/glad.h"
#include "core/game_window.h"
#include "core/event.h"
#include "graphics/sprite_batch.h"
#include "graphics/texture_2d.h"
#include "spdlog/spdlog.h"

#include "gui/imgui.h"
#include "gui/imgui_impl_sdl.h"
#include "gui/imgui_impl_opengl3.h"

int main() {
  spdlog::info("Initializing game...");
  const int viewport_width = 1280;
  const int viewport_height = 720;

  auto window = GameWindow("Kingdom", viewport_width, viewport_height);
  auto spriteBatch = SpriteBatch(viewport_width, viewport_height);
  auto texture = Texture2D("content/textures/placeholder.png");

  // ImGui testing
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  ImGui::StyleColorsDark();

  ImGui_ImplSDL2_InitForOpenGL(window.Handle(), window.GLContext());
  ImGui_ImplOpenGL3_Init("#version 400");

  ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.6f, 1.f);

  // fixed step logic
  float timer = window.Dt();
  const float fixed_step = 0.01667f; // 1/60th of a second

  SDL_Event sdlEvent;
  while (true) {
    if (SDL_PollEvent(&sdlEvent)) {
      ImGui_ImplSDL2_ProcessEvent(&sdlEvent);
      if (sdlEvent.type == SDL_QUIT) break;
      if (sdlEvent.type == SDL_WINDOWEVENT && sdlEvent.window.event == SDL_WINDOWEVENT_CLOSE) break;
      if (sdlEvent.type == SDL_MOUSEMOTION) {
        gEventHandler.Post(MouseMoveEvent(sdlEvent.motion.x, sdlEvent.motion.y));
      }
    }

    // ImGui testing
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window.Handle());
    ImGui::NewFrame();

    ImGui::Begin("Hello, world!");
    ImGui::Text("This is some useful text.");
    //ImGui::Checkbox("Demo Window", nullptr);
    //ImGui::Checkbox("Another Window", nullptr);
    ImGui::End();

    // Update logic
    timer += window.Dt();
    if (timer >= fixed_step) {
      timer -= fixed_step;
    }

    // Rendering logic
    ImGui::Render();
    glClear(GL_COLOR_BUFFER_BIT);
    
    auto pos = glm::vec2(640.f - texture.Width() / 2.f, 360 - texture.Height() / 2.f);
    spriteBatch.Draw(texture, pos);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    window.SwapBuffers();
  }

  gEventHandler.Post(ApplicationCloseEvent());
  spdlog::info("Program execution finished.");
  return 0;
}
