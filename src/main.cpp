#include <iostream>
#include "glad/glad.h"
#include "core/game_window.h"
#include "core/event.h"
#include "graphics/sprite_batch.h"
#include "graphics/texture_2d.h"
#include "spdlog/spdlog.h"
#include "entt/entt.hpp"
#include "math/rng.h"
#include "gui/imgui.h"
#include "gui/imgui_impl_sdl.h"
#include "gui/imgui_impl_opengl3.h"
#include "game/world_data.h"

struct Transform {
  glm::vec2 pos;
  glm::vec2 scale;
  float rotation;
};

struct Velocity {
  glm::vec2 vel;
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

  /*for (int i = 0; i < 500; i++) {
    auto entity = registry.create();
    registry.emplace<Transform>(entity, glm::vec2(0.f), glm::vec2(0.1f), 0.f);
    registry.emplace<Velocity>(entity, glm::vec2(rng::next_int(-100, 100), rng::next_int(-100, 100)));
    registry.emplace<TextureData>(entity, &texture);
  }*/

  auto world = registry.create();
  registry.emplace<WorldData>(world, 5, 5);

  auto& world_data = registry.get<WorldData>(world);
  for (int i = 0; i < world_data.width * world_data.height; i++) {
    world_data.tileData[i] = rng::next_int(0, 2);
    spdlog::info("{} = {}", i, world_data.tileData[i]);
  }

  auto renderView = registry.view<Transform, TextureData>();
  auto physicsView = registry.view<Transform, Velocity>();

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

      for (auto& entity : physicsView) {
        auto& transform = physicsView.get<Transform>(entity);
        auto& velocity = physicsView.get<Velocity>(entity);

        transform.pos += velocity.vel * fixed_step;

        if (transform.pos.x < 0.f || transform.pos.x > 1230.f) velocity.vel.x *= -1.f;
        if (transform.pos.y < 0.f || transform.pos.y > 670.f) velocity.vel.y *= -1.f;
      }
    }

    // Rendering logic
    ImGui::Render();
    glClear(GL_COLOR_BUFFER_BIT);

    // Renderable entity rendering
    spriteBatch.Begin(texture);
    for (auto& entity : renderView) {
      auto& transform = renderView.get<Transform>(entity);
      auto& tData = renderView.get<TextureData>(entity).texture;

      spriteBatch.Draw(*tData, transform.pos, transform.scale);
    }
    spriteBatch.End();

    // ImGui rendering
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    window.SwapBuffers();
  }

  gEventHandler.Post(ApplicationCloseEvent());
  spdlog::info("Program execution finished.");
  return 0;
}
