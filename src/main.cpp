#include <iostream>
#include "glad/glad.h"
#include "core/game_window.h"
#include "core/event.h"
#include "graphics/sprite_batch.h"
#include "spdlog/spdlog.h"
#include "entt/entt.hpp"
#include "math/rng.h"
#include "gui/imgui.h"
#include "gui/imgui_impl_sdl.h"
#include "gui/imgui_impl_opengl3.h"
#include "game/world_data.h"
#include "content/file_handler.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "math/perlin.h"
#include "graphics/tile_sheet.h"
#include "math/func.h"

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

  // Test sprite batch entities
  /*for (int i = 0; i < 500; i++) {
    auto entity = registry.create();
    registry.emplace<Transform>(entity, glm::vec2(0.f), glm::vec2(0.1f), 0.f);
    registry.emplace<Velocity>(entity, glm::vec2(rng::next_int(-100, 100), rng::next_int(-100, 100)));
    registry.emplace<TextureData>(entity, &texture);
  }*/

  TileSheet tilesheet("content/textures/tileset.png", 32);

  auto world = registry.create();
  registry.emplace<WorldData>(world, 160, 92);

  // generate test world data
  Perlin perlin(time(nullptr));

  auto& world_data = registry.get<WorldData>(world);
  int index = 0;
  for (int y = 0; y < world_data.height; y++) {
    for (int x = 0; x < world_data.width; x++) {

      float xf = float(x) / float(world_data.width);
      float yf = float(y) / float(world_data.height);

      float sSample = perlin.Noise(10.f * xf, 10.f * yf, 1.f);
      float tSample = perlin.Noise(10.f * xf, 10.f * yf, 0.f);

      world_data.tileData[index++] = (sSample >= 0.75f) ? 1 : (tSample >= 0.6f) ? 2 : (tSample >= 0.55f) ? 3 : 0;
    }
  }

  auto *buffer = new float[world_data.width * world_data.height * 24];
  const float tileSize = 32.f;

  index = 0;
  for (int y = 0; y < world_data.height; y++) {
    for (int x = 0; x < world_data.width; x++) {
      const int tileIndex = world_data.tileData[y * world_data.width + x];
      const auto point = glm::vec2(
        float(tileIndex % tilesheet.Width()) / tilesheet.Width(),
        float(tileIndex / tilesheet.Width()) / tilesheet.Height()
      );
      const auto size = glm::vec2(1.f / tilesheet.Width(), 1.f / tilesheet.Height());

      buffer[index++] = x * tileSize;
      buffer[index++] = y * tileSize;
      buffer[index++] = point.x;
      buffer[index++] = point.y;

      buffer[index++] = x * tileSize;
      buffer[index++] = y * tileSize + tileSize;
      buffer[index++] = point.x;
      buffer[index++] = point.y + size.y;

      buffer[index++] = x * tileSize + tileSize;
      buffer[index++] = y * tileSize;
      buffer[index++] = point.x + size.x;
      buffer[index++] = point.y;

      buffer[index++] = x * tileSize + tileSize;
      buffer[index++] = y * tileSize;
      buffer[index++] = point.x + size.x;
      buffer[index++] = point.y;

      buffer[index++] = x * tileSize;
      buffer[index++] = y * tileSize + tileSize;
      buffer[index++] = point.x;
      buffer[index++] = point.y + size.y;

      buffer[index++] = x * tileSize + tileSize;
      buffer[index++] = y * tileSize + tileSize;
      buffer[index++] = point.x + size.x;
      buffer[index++] = point.y + size.y;
    }
  }

  VertexBuffer vBuffer;

  vBuffer.Bind();
  vBuffer.SetBufferData(buffer, sizeof(float) * world_data.width * world_data.height * 24);

  vBuffer.EnableVertexAttribute(0); // Position
  vBuffer.EnableVertexAttribute(1); // UV

  vBuffer.VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)0);
  vBuffer.VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)(2 * sizeof(float)));
  vBuffer.Unbind();

  ShaderProgram sProgram;
  sProgram.LoadShaderSources(
    ReadTextFile("content/shaders/texturedVS.glsl"),
    ReadTextFile("content/shaders/texturedFS.glsl")
  );

  glm::mat4 proj, view, model;
  proj = glm::ortho(0.f, float(window.Width()) * 2.f, float(window.Height()) * 2.f, 0.f, -1.f, 1.f);
  view = glm::mat4(1.f);
  model = glm::mat4(1.f);

  sProgram.Use();
  sProgram.SetUniform("proj", proj);
  sProgram.SetUniform("view", view);
  sProgram.SetUniform("model", model);
  sProgram.SetUniform("tint", glm::vec3(1.f));

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
  float dayTimer = 0.f;
  auto tint = glm::vec3(1.f);

  const glm::vec3 nightColor = glm::vec3(47.f, 43.f, 94.f) / glm::vec3(255.f);

  const float fixed_step = 0.01667f; // 1/60th of a second

  bool demo_window = false;
  bool another_window = true;

  SDL_Event sdlEvent;
  while (true) {
    // Event Handling
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

    // Update logic
    timer += window.Dt();
    if (timer >= fixed_step) {
      timer -= fixed_step;
      //dayTimer += fixed_step;

      /*const float dayLength = 5.f;
      if (dayTimer >= dayLength * 2.f) dayTimer = 0.f;
      if (dayTimer >= 0.f && dayTimer <= dayLength) {
        tint.r = lerp(1.f, nightColor.r, dayTimer / dayLength);
        tint.g = lerp(1.f, nightColor.g, dayTimer / dayLength);
        tint.b = lerp(1.f, nightColor.b, dayTimer / dayLength);
      } else {
        tint.r = lerp(nightColor.r, 1.f, (dayTimer - dayLength) / dayLength);
        tint.g = lerp(nightColor.g, 1.f, (dayTimer - dayLength) / dayLength);
        tint.b = lerp(nightColor.b, 1.f, (dayTimer - dayLength) / dayLength);
      }*/

      sProgram.SetUniform("tint", tint);

      /*for (auto& entity : physicsView) {
        auto& transform = physicsView.get<Transform>(entity);
        auto& velocity = physicsView.get<Velocity>(entity);

        transform.pos += velocity.vel * fixed_step;

        if (transform.pos.x < 0.f || transform.pos.x > 1230.f) velocity.vel.x *= -1.f;
        if (transform.pos.y < 0.f || transform.pos.y > 670.f) velocity.vel.y *= -1.f;
      }*/
    }

    // Rendering logic
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window.Handle());
    ImGui::NewFrame();

    ImGui::Begin("Hello, world!");
    ImGui::Text("This is some useful text.");
    ImGui::Checkbox("Demo Window", &demo_window);
    ImGui::Checkbox("Another Window", &another_window);
    ImGui::DragFloat("Time", &dayTimer, 0.f, 10.f);
    ImGui::DragFloat("R", &tint.r, 0.f, 1.f);
    ImGui::DragFloat("G", &tint.g, 0.f, 1.f);
    ImGui::DragFloat("B", &tint.b, 0.f, 1.f);
    ImGui::End();

    ImGui::Render();

    glClear(GL_COLOR_BUFFER_BIT);

    // Renderable entity rendering
    /*spriteBatch.Begin(texture);
    for (auto& entity : renderView) {
      auto& transform = renderView.get<Transform>(entity);
      auto& tData = renderView.get<TextureData>(entity).texture;

      spriteBatch.Draw(*tData, transform.pos, transform.scale);
    }
    spriteBatch.End();*/

    vBuffer.Bind();
    tilesheet.Bind();
    glDrawArrays(GL_TRIANGLES, 0, world_data.width * world_data.height * 6);
    vBuffer.Unbind();

    // ImGui rendering
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    window.SwapBuffers();
  }

  gEventHandler.Post(ApplicationCloseEvent());
  spdlog::info("Program execution finished.");
  return 0;
}
