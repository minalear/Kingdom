#include <iostream>
#include "glad/glad.h"
#include "core/game_window.h"
#include "core/event.h"
#include "graphics/sprite_batch.h"
#include "graphics/texture_2d.h"
#include "spdlog/spdlog.h"
#include "entt/entt.hpp"

#include "gui/imgui.h"
#include "gui/imgui_impl_sdl.h"
#include "gui/imgui_impl_opengl3.h"

struct Transform {
  glm::vec2 pos;
  glm::vec2 scale;
  float rotation;
};

struct TextureData {
  Texture2D* texture;
};

int main() {
  spdlog::info("Initializing game...");
  const int viewport_width = 1280;
  const int viewport_height = 720;

  auto window = GameWindow("Kingdom", viewport_width, viewport_height);
  auto spriteBatch = SpriteBatch(viewport_width, viewport_height);
  auto texture = Texture2D("content/textures/placeholder.png");

  entt::registry registry;
  auto entity = registry.create();
  registry.emplace<Transform>(entity, glm::vec2(0.f), glm::vec2(1.f), 0.f);
  registry.emplace<TextureData>(entity, &texture);

  auto view = registry.view<Transform, TextureData>();

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

  bool demo_window = false;
  bool another_window = true;

  SDL_Event sdlEvent;
  while (true) {
    if (SDL_PollEvent(&sdlEvent)) {
      ImGui_ImplSDL2_ProcessEvent(&sdlEvent);
      if (sdlEvent.type == SDL_QUIT) break;
      else if (sdlEvent.type == SDL_WINDOWEVENT && sdlEvent.window.event == SDL_WINDOWEVENT_CLOSE) break;
      else if (sdlEvent.type == SDL_MOUSEMOTION) {
        gEventHandler.Post(MouseMoveEvent(sdlEvent.motion.x, sdlEvent.motion.y));
      }
      else if (sdlEvent.type == SDL_KEYUP) {
        gEventHandler.Post(KeyboardEvent(EventType::KeyboardButtonUp, sdlEvent.key.keysym.scancode));
      }
      else if (sdlEvent.type == SDL_KEYDOWN) {
        gEventHandler.Post(KeyboardEvent(EventType::KeyboardButtonDown, sdlEvent.key.keysym.scancode));
      }
    }

    // ImGui testing
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window.Handle());
    ImGui::NewFrame();

    /*ImGui::Begin("Hello, world!");
    ImGui::Text("This is some useful text.");
    ImGui::Checkbox("Demo Window", &demo_window);
    ImGui::Checkbox("Another Window", &another_window);
    ImGui::End();*/

    // Update logic
    timer += window.Dt();
    if (timer >= fixed_step) {
      timer -= fixed_step;


    }

    // Rendering logic
    ImGui::Render();
    glClear(GL_COLOR_BUFFER_BIT);

    for (auto& entity : view) {
      auto& transform = view.get<Transform>(entity);
      auto& texture = view.get<TextureData>(entity).texture;

      spriteBatch.Draw(*texture, transform.pos, transform.scale);
    }

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    window.SwapBuffers();
  }

  gEventHandler.Post(ApplicationCloseEvent());
  spdlog::info("Program execution finished.");
  return 0;
}
