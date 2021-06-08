#include <iostream>
#include <cmath>
#include "glad/glad.h"
#include "core/game_window.h"
#include "core/event.h"
#include "spdlog/spdlog.h"
#include "entt/entt.hpp"
#include "gui/imgui_impl_sdl.h"
#include "math/perlin.h"
#include "graphics/texture_2d.h"
#include "graphics/sprite_batch.h"
#include "gui/imgui.h"
#include "gui/imgui_impl_sdl.h"
#include "gui/imgui_impl_opengl3.h"

const int mapWidth = 1280;
const int mapHeight = 720;

auto islandData = new float[mapWidth * mapHeight];

float distance(float x1, float y1, float x2, float y2) {
  return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

void generateTextureData(const Texture2D& image) {
    auto newData = new uint8_t[mapWidth * mapHeight * 4]; // 4 bytes per pixel

  for (size_t i = 0; i < mapWidth * mapHeight; i++) {
    auto value = uint8_t(255 * islandData[i]);
    newData[i * 4 + 0] = value;
    newData[i * 4 + 1] = value;
    newData[i * 4 + 2] = value;
    newData[i * 4 + 3] = value;
  }

  image.SetTextureData(newData);
  delete[] newData;
}

void generateIsland(const Perlin& perlin, const Texture2D& image) {
  // reset map to zero
  for (size_t i = 0; i < mapWidth * mapHeight; i++) {
    islandData[i] = 0;
  }

  // create basic circle
  const int cx = mapWidth / 2;
  const int cy = mapHeight / 2;
  const float maxDistance = distance(0, 0, cx, cy);

  for (size_t i = 0; i < mapWidth * mapHeight; i++) {
    const int x = i % mapWidth;
    const int y = i / mapWidth;

    float value = 1.f - (distance(x, y, cx, cy) / maxDistance);
    if (value <= 0.45f) value = 0.f;

    islandData[i] = value;
  }

  // layer noise to create randomness
  for (size_t i = 0; i < mapWidth * mapHeight; i++) {
    const int x = i % mapWidth;
    const int y = i / mapWidth;

    // perlin layers
    float xf = float(x) / float(mapWidth);
    float yf = float(y) / float(mapHeight);

    float n1 = islandData[i];
    float n2 = perlin.Noise(xf, yf, 0.f);
    float n3 = perlin.Noise(5.f * xf, 5.f * yf, 0.f);
    float n4 = perlin.Noise(10.f * xf, 10.f * yf, 0.f);

    float value = n1 * (n2 + n3 + n4) / 3.f;
    if (value <= 0.3f) value = 0.f;
    islandData[i] = value;
  }

  generateTextureData(image);
}

int main() {
  spdlog::info("Initializing game...");
  const int viewport_width = 1280;
  const int viewport_height = 720;

  auto window = GameWindow("Kingdom", viewport_width, viewport_height);

  // fixed step logic
  float timer = window.Dt();
  const float fixed_step = 0.01667f; // 1/60th of a second

  // Perlin noise generator and texture
  Perlin perlin(time(nullptr));

  auto perlinImage = Texture2D(mapWidth, mapHeight);
  generateIsland(perlin, perlinImage);

  // SpriteBatch is used to render our image to screen
  SpriteBatch sb(viewport_width, viewport_height);

  // Debug GUI
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  ImGui::StyleColorsClassic();

  ImGui_ImplSDL2_InitForOpenGL(window.Handle(), window.GLContext());
  ImGui_ImplOpenGL3_Init("#version 400");
  ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.6f, 1.f);

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

      // do fixed-step logic here with fixed_step as dt
    }

    // Gui
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window.Handle());
    ImGui::NewFrame();

    ImGui::Begin("Perlin Generator");
    ImGui::SliderFloat("A", &A, 0.1f, 2.f);
    ImGui::SliderFloat("B", &B, 0.1f, 20.f);
    ImGui::SliderFloat("C", &C, 0.1f, 20.f);
    ImGui::SliderFloat("Z", &Z, 0.1f, 20.f);
    ImGui::SliderFloat("Limit", &lim, 0.f, 1.f);
    if (ImGui::Button("Generate"))
      generateIsland(perlin, perlinImage);
    if (ImGui::Button("New Seed")) {
      perlin.SetNewSeed(time(nullptr));
      generateIsland(perlin, perlinImage);
    }
    ImGui::End();

    ImGui::Render();
    glClear(GL_COLOR_BUFFER_BIT);
    sb.Begin(perlinImage);
    sb.Draw(perlinImage, glm::vec2(0.f), glm::vec2(1.f));
    sb.End();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    window.SwapBuffers();
  }

  gEventHandler.Post(ApplicationCloseEvent());
  spdlog::info("Program execution finished.");
  return 0;
}
