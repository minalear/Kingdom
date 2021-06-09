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
#include "content/file_handler.h"
#include "graphics/shader_program.h"
#include "graphics/vertex_buffer.h"
#include "game/world_data.h"
#include "graphics/tile_sheet.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

const int mapWidth = 128;
const int mapHeight = 72;

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

  // world entity and tile data setup
  entt::registry registry;
  auto world = registry.create();
  registry.emplace<WorldData>(world, mapWidth, mapHeight);

  auto& worldData = registry.get<WorldData>(world);

  // Initial landmass setup
  for (size_t i = 0; i < mapWidth * mapHeight; i++) {
    worldData.tileData[i] = (islandData[i] == 0.f) ? 0 : 1;
  }

  // Coordinate bitmasks
  uint8_t NW = 0b00000001;
  uint8_t  N = 0b00000010;
  uint8_t NE = 0b00000100;
  uint8_t  W = 0b00001000;
  uint8_t  E = 0b00010000;
  uint8_t SW = 0b00100000;
  uint8_t  S = 0b01000000;
  uint8_t SE = 0b10000000;

  // map for landmass border
  std::map<uint8_t, int> tilesetIndices;
  tilesetIndices.insert(std::pair(0b00000000, 12)); // all water
  tilesetIndices.insert(std::pair(0b11111111, 1)); // all land
  tilesetIndices.insert(std::pair(N, 2));
  tilesetIndices.insert(std::pair(S, 3));
  tilesetIndices.insert(std::pair(E, 4));
  tilesetIndices.insert(std::pair(W, 5));
  tilesetIndices.insert(std::pair(E | S | SE, 6));
  tilesetIndices.insert(std::pair(W | S | SW, 7));
  tilesetIndices.insert(std::pair(N | E | NE, 18));
  tilesetIndices.insert(std::pair(W | N | NW, 19));
  tilesetIndices.insert(std::pair(N | E | S | NE | SE, 13));
  tilesetIndices.insert(std::pair(N | W | S | NW | SW, 14));
  tilesetIndices.insert(std::pair(W | N | E | NW | NE, 15));
  tilesetIndices.insert(std::pair(W | S | E | SW | SE, 16));
  tilesetIndices.insert(std::pair(N | S | E | W, 17));
  tilesetIndices.insert(std::pair(W | E | S, 24));
  tilesetIndices.insert(std::pair(W | E | N, 25));
  tilesetIndices.insert(std::pair(W | N | S, 26));
  tilesetIndices.insert(std::pair(E | N | S, 27));
  tilesetIndices.insert(std::pair(N | S | E | W | NE | SW, 28));
  tilesetIndices.insert(std::pair(N | S | E | W | NW | SE, 29));
  tilesetIndices.insert(std::pair(N | S, 30));
  tilesetIndices.insert(std::pair(E | W, 31));
  tilesetIndices.insert(std::pair(E | S, 36));
  tilesetIndices.insert(std::pair(W | S, 37));
  tilesetIndices.insert(std::pair(N | E, 48));
  tilesetIndices.insert(std::pair(N | W, 49));
  tilesetIndices.insert(std::pair(N | S | E | W | SW | SE | NE, 38));
  tilesetIndices.insert(std::pair(N | S | E | W | NW | SW | SE, 39));
  tilesetIndices.insert(std::pair(N | S | E | W | NW | NE | SE, 50));
  tilesetIndices.insert(std::pair(N | S | E | W | NW | NE | SW, 51));
  tilesetIndices.insert(std::pair(N | S | E | SE, 40));
  tilesetIndices.insert(std::pair(N | S | E | NE, 41));
  tilesetIndices.insert(std::pair(N | S | W | SW, 42));
  tilesetIndices.insert(std::pair(N | S | W | NW, 43));
  tilesetIndices.insert(std::pair(N | E | W | NW, 52));
  tilesetIndices.insert(std::pair(N | E | W | NE, 53));
  tilesetIndices.insert(std::pair(E | W | S | SW, 54));
  tilesetIndices.insert(std::pair(E | W | S | SE, 55));
  tilesetIndices.insert(std::pair(N | S | E | W | SE, 60));
  tilesetIndices.insert(std::pair(N | S | E | W | SW, 61));
  tilesetIndices.insert(std::pair(N | S | E | W | NE, 62));
  tilesetIndices.insert(std::pair(N | S | E | W | NW, 63));
  tilesetIndices.insert(std::pair(N | S | E | W | NE | NW, 64));
  tilesetIndices.insert(std::pair(N | S | E | W | SE | SW, 65));
  tilesetIndices.insert(std::pair(N | S | E | W | NE | SE, 66));
  tilesetIndices.insert(std::pair(N | S | E | W | NW | SW, 67));
  const int errorTile = 8;
  const int mountainTile = 9;
  const int forestTile = 10;
  const int treeTile = 22;
  const int rockTile = 11;
  const int wheatTile = 20;
  const int waterRockTile = 44;

  // Landmass border generation
  for (size_t i = 0; i < mapWidth * mapHeight; i++) {
    const int tileIndex = worldData.GetTileIndex(i);
    if (tileIndex == 0) continue; // ignore water

    // get coordinates
    const int x = i % mapWidth;
    const int y = i / mapWidth;

    // calculate bitmask for tile's neighbors
    const int nw = worldData.GetTileIndex(x - 1, y - 1) == 0 ? 0 : 1;
    const int  n = worldData.GetTileIndex(x, y - 1) == 0 ? 0 : 1;
    const int ne = worldData.GetTileIndex(x + 1, y - 1) == 0 ? 0 : 1;
    const int  w = worldData.GetTileIndex(x - 1, y) == 0 ? 0 : 1;
    const int  e = worldData.GetTileIndex(x + 1, y) == 0 ? 0 : 1;
    const int sw = worldData.GetTileIndex(x - 1, y + 1) == 0 ? 0 : 1;
    const int  s = worldData.GetTileIndex(x, y + 1) == 0 ? 0 : 1;
    const int se = worldData.GetTileIndex(x + 1, y + 1) == 0 ? 0 : 1;

    uint8_t bitmask = 0x00;
    if (n && w) bitmask +=   1 * nw;
                bitmask +=   2 * n;
    if (n && e) bitmask +=   4 * ne;
                bitmask +=   8 * w;
                bitmask +=  16 * e;
    if (s && w) bitmask +=  32 * sw;
                bitmask +=  64 * s;
    if (s && e) bitmask += 128 * se;

    //spdlog::info("tile {} - bitmask {:b}", i, bitmask);

    if (tilesetIndices.find(bitmask) == tilesetIndices.end()) {
      worldData.SetTileIndex(i, errorTile);
    } else {
      worldData.SetTileIndex(i, tilesetIndices[bitmask]);
    }
  }

  // Tree, Mountain, and Rock Generation
  for (size_t i = 0; i < mapWidth * mapHeight; i++) {
    const auto tileIndex = worldData.GetTileIndex(i);

    const int x = i % mapWidth;
    const int y = i / mapWidth;
    const float xf = float(x) / float(mapWidth);
    const float yf = float(y) / float(mapHeight);

    int newIndex = tileIndex;

    if (tileIndex == 1) {
      // land features
      const float mountainSample = perlin.Noise(10.f * xf, 10.f * yf, 0.f);
      const float rockSample = perlin.Noise(10.f * xf, 10.f * yf, 1.f);
      const float treeSample = perlin.Noise(10.f * xf, 10.f * yf, 2.f);
      const float wheatSample = perlin.Noise(10.f * xf, 10.f * yf, 3.f);

      newIndex = (mountainSample >= 0.75f) ? mountainTile :
                           (rockSample >= 0.75f) ? rockTile :
                           (treeSample >= 0.6f) ? forestTile :
                           (treeSample >= 0.55f) ? treeTile :
                           (wheatSample >= 0.4f) ? wheatTile : tileIndex;
    } else if (tileIndex == 0) {
      // water features
      const float waterRockSample = perlin.Noise(10.f * xf, 10.f * yf, 0.f);
      newIndex = (waterRockSample >= 0.85f) ? waterRockTile : tileIndex;
    }

    worldData.SetTileIndex(i, newIndex);
  }

  // world rendering setup
  auto tilesheet = TileSheet("content/textures/tileset.png", 32);
  auto* buffer = new float[worldData.width * worldData.height * 24];
  const float tileSize = 32.f;
  const auto size = glm::vec2(1.f / tilesheet.Width(), 1.f / tilesheet.Height());

  for (size_t i = 0; i < mapWidth * mapHeight; i++) {
    const auto tileIndex = worldData.GetTileIndex(i);
    const auto point = glm::vec2(
  float(tileIndex % tilesheet.Width()) / tilesheet.Width(),
  float(tileIndex / tilesheet.Width()) / tilesheet.Height()
    );
    const size_t x = i % mapWidth;
    const size_t y = i / mapWidth;

    buffer[i * 24 + 0] = x * tileSize;
    buffer[i * 24 + 1] = y * tileSize;
    buffer[i * 24 + 2] = point.x;
    buffer[i * 24 + 3] = point.y;

    buffer[i * 24 + 4] = x * tileSize;
    buffer[i * 24 + 5] = y * tileSize + tileSize;
    buffer[i * 24 + 6] = point.x;
    buffer[i * 24 + 7] = point.y + size.y;

    buffer[i * 24 + 8] = x * tileSize + tileSize;
    buffer[i * 24 + 9] = y * tileSize;
    buffer[i * 24 + 10] = point.x + size.x;
    buffer[i * 24 + 11] = point.y;


    buffer[i * 24 + 12] = x * tileSize + tileSize;
    buffer[i * 24 + 13] = y * tileSize;
    buffer[i * 24 + 14] = point.x + size.x;
    buffer[i * 24 + 15] = point.y;

    buffer[i * 24 + 16] = x * tileSize;
    buffer[i * 24 + 17] = y * tileSize + tileSize;
    buffer[i * 24 + 18] = point.x;
    buffer[i * 24 + 19] = point.y + size.y;

    buffer[i * 24 + 20] = x * tileSize + tileSize;
    buffer[i * 24 + 21] = y * tileSize + tileSize;
    buffer[i * 24 + 22] = point.x + size.x;
    buffer[i * 24 + 23] = point.y + size.y;
  }

  VertexBuffer vBuffer;
  vBuffer.Bind();
  vBuffer.SetBufferData(buffer, sizeof(float) * mapWidth * mapHeight * 24);
  vBuffer.EnableVertexAttribute(0);
  vBuffer.EnableVertexAttribute(1);
  vBuffer.VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)0);
  vBuffer.VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)(2 * sizeof(float)));
  vBuffer.Unbind();

  auto shaderProgram = ShaderProgram(
      ReadTextFile("content/shaders/texturedVS.glsl"),
      ReadTextFile("content/shaders/texturedFS.glsl")
  );

  glm::mat4 proj, view, model;
  proj = glm::ortho(0.f, float(window.Width()), float(window.Height()), 0.f, -1.f, 1.f);
  view = glm::scale(glm::mat4(1.f), glm::vec3(0.5f));
  model = glm::mat4(1.f);

  shaderProgram.Use();
  shaderProgram.SetUniform("proj", proj);
  shaderProgram.SetUniform("model", model);
  shaderProgram.SetUniform("view", view);

  // Debug GUI
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  ImGui::StyleColorsClassic();

  ImGui_ImplSDL2_InitForOpenGL(window.Handle(), window.GLContext());
  ImGui_ImplOpenGL3_Init("#version 400");
  ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.6f, 1.f);

  bool mouseDown = false;
  auto cameraPos = glm::vec2(0.f);
  auto oldPos = glm::vec2(0.f);
  auto newPos = glm::vec2(0.f);

  SDL_Event sdlEvent;
  while (true) {
    // Event Handling
    if (SDL_PollEvent(&sdlEvent)) {
      ImGui_ImplSDL2_ProcessEvent(&sdlEvent);
      if (sdlEvent.type == SDL_QUIT) break;
      else if (sdlEvent.type == SDL_WINDOWEVENT && sdlEvent.window.event == SDL_WINDOWEVENT_CLOSE) break;
      else if (sdlEvent.type == SDL_KEYUP) {
        gEventHandler.Post(KeyboardEvent(EventType::KeyboardButtonUp, sdlEvent.key.keysym.scancode));
      }
      else if (sdlEvent.type == SDL_KEYDOWN) {
        gEventHandler.Post(KeyboardEvent(EventType::KeyboardButtonDown, sdlEvent.key.keysym.scancode));
      }

      if (sdlEvent.type == SDL_MOUSEBUTTONDOWN && sdlEvent.button.button == SDL_BUTTON_LEFT) {
        mouseDown = true;
        oldPos = newPos = glm::vec2(sdlEvent.button.x, sdlEvent.button.y);
      } else if (sdlEvent.type == SDL_MOUSEBUTTONUP && sdlEvent.button.button == SDL_BUTTON_LEFT) {
        mouseDown = false;
      }

      if (sdlEvent.type == SDL_MOUSEMOTION) {
        gEventHandler.Post(MouseMoveEvent(sdlEvent.motion.x, sdlEvent.motion.y));

        if (mouseDown) {
          oldPos = newPos;
          newPos = glm::vec2(sdlEvent.motion.x, sdlEvent.motion.y);

          cameraPos += newPos - oldPos;

          auto camera = glm::scale(glm::mat4(1.f), glm::vec3(0.5f));
          camera = glm::translate(camera, glm::vec3(cameraPos, 0.f));
          shaderProgram.Use();
          shaderProgram.SetUniform("view", camera);
        }
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
    if (ImGui::Button("Generate"))
      generateIsland(perlin, perlinImage);
    if (ImGui::Button("New Seed")) {
      perlin.SetNewSeed(time(nullptr));
      generateIsland(perlin, perlinImage);
    }
    ImGui::End();

    ImGui::Render();
    glClear(GL_COLOR_BUFFER_BIT);
    /*sb.Begin(perlinImage);
    sb.Draw(perlinImage, glm::vec2(0.f), glm::vec2(8.f));
    sb.End();*/

    vBuffer.Bind();
    tilesheet.Bind();
    glDrawArrays(GL_TRIANGLES, 0, mapWidth * mapHeight * 6);
    vBuffer.Unbind();

    //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    window.SwapBuffers();
  }

  gEventHandler.Post(ApplicationCloseEvent());
  spdlog::info("Program execution finished.");
  return 0;
}
