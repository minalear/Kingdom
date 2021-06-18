#include <iostream>
#include <cmath>
#include "glad/glad.h"
#include "core/game_window.h"
#include "core/event.h"
#include "spdlog/spdlog.h"
#include "entt/entt.hpp"
#include "gui/imgui_impl_sdl.h"
#include "math/perlin.h"
#include "math/rng.h"
#include "graphics/texture_2d.h"
#include "graphics/sprite_batch.h"
#include "gui/imgui.h"
#include "gui/imgui_impl_sdl.h"
#include "gui/imgui_impl_opengl3.h"
#include "content/file_handler.h"
#include "graphics/shader_program.h"
#include "graphics/vertex_buffer.h"
#include "game/world_data.h"
#include "game/world_generator.h"
#include "graphics/tile_sheet.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "toml.hpp"
#include "content/tinyxml2.h"

/*void generateTextureData(const Texture2D& image) {
  auto heightmap = GenerateHeightmap(time(nullptr), mapWidth, mapHeight);
  auto imageData = new uint8_t[mapWidth * mapHeight * 4];

  for (size_t i = 0; i < mapWidth * mapHeight; i++) {
    const float height = heightmap[i];
    const uint8_t stepHeight =
        (height <= waterLevel) ? 0 :
        (height <= mountainLevel) ? 1 : 2;

    auto value = 100 * stepHeight;
    imageData[i * 4 + 0] = value;
    imageData[i * 4 + 1] = value;
    imageData[i * 4 + 2] = value;
    imageData[i * 4 + 3] = 255;
  }
  image.SetTextureData(imageData);
  delete[] imageData;
}*/

template <typename T>
bool validBlock(const std::vector<T>& vec, size_t i0, T value, int width, int height) {
  const int x = i0 % width;
  const int y = i0 / height;

  // get indices of the 2x2 block
  const size_t i1 = i0 + 1;
  const size_t i2 = i0 + width;
  const size_t i3 = i2 + 1;

  const size_t len = vec.size();
  if (i0 >= len || i1 >= len || i2 >= len || i3 >= len) return false;
  if (i0 < 0 || i1 <= 0 || i2 <= 0 || i3 <= 0) return false;

  return (vec[i0] == value && vec[i1] == value && vec[i2] == value && vec[i3] == value);
}

template <typename T>
void setBlock(std::vector<T>& vec, size_t i0, T value, int width, int height) {
  const int x = i0 % width;
  const int y = i0 / height;

  // get indices of the 2x2 block
  const size_t i1 = i0 + 1;
  const size_t i2 = i0 + width;
  const size_t i3 = i2 + 1;

  vec[i0] = vec[i1] = vec[i2] = vec[i3] = value;
}

template <typename T>
T getValueByCoord(const std::vector<T>& vec, int x, int y, int width) {
  const size_t index = x + y * width;
  if (index < 0 || index >= vec.size()) return -1;
  return vec[index];
}

template <typename T>
uint8_t calculateBitmask(const std::vector<T>& vec, size_t i, int width) {
  const int x = i % width;
  const int y = i / width;

  uint8_t bitmask = 0x00;
  const int n = getValueByCoord(vec, x, y - 1, width);
  const int e = getValueByCoord(vec, x + 1, y, width);
  const int s = getValueByCoord(vec, x, y + 1, width);
  const int w = getValueByCoord(vec, x - 1, y, width);

  bitmask += 1 * n;
  bitmask += 2 * e;
  bitmask += 4 * s;
  bitmask += 8 * w;

  return bitmask;
}

int main() {
  spdlog::info("Initializing game...");

  const int viewport_width = 1280;
  const int viewport_height = 720;

  auto window = GameWindow("Kingdom", viewport_width, viewport_height);

  // XML parsing test
  /*tinyxml2::XMLDocument doc;
  doc.LoadFile("content/data/tileset.tsx");

  auto* tilesetNode = doc.FirstChildElement("tileset");
  int tilewidth = tilesetNode->IntAttribute("tilewidth");
  int tileheight = tilesetNode->IntAttribute("tileheight");
  int tilecount = tilesetNode->IntAttribute("tilecount");
  int tilecolumns = tilesetNode->IntAttribute("columns");

  spdlog::info("width: {}, height: {}, count: {}, columns: {}",
               tilewidth, tileheight, tilecount, tilecolumns);*/

  // TOML testing
  /*auto tilesetData = toml::parse("content/data/tileset.data");
  auto metaData = toml::find(tilesetData, "meta");
  spdlog::info("Tileset Loading: path {}", toml::find<std::string>(metaData, "path"));

  auto autotileData = toml::find(tilesetData, "autotile");
  auto bitmaskData = toml::find<toml::array>(autotileData, "bitmasks");

  // load bitmask data from toml file
  std::map<uint8_t, int> tilesetIndices;
  for (auto& i : bitmaskData) {
    const auto bitmask = uint8_t(toml::get<int>(i[0]));
    const auto tileIndex = int(toml::get<int>(i[1]));

    tilesetIndices.insert(std::pair(bitmask, tileIndex));
  }*/

  // Generate map
  const int mapWidth = 256;
  const int mapHeight = 144;
  const int mapDepth = 2;

  float seaLevel = 0.4f;
  float mountainLevel = 0.6f;

  auto heightmap = GenerateHeightmap(time(nullptr), mapWidth, mapHeight);
  for (auto& value : heightmap) {
    value = (value <= seaLevel) ? 0.f :
            (value <= mountainLevel) ? 1.f : 2.f;
  }

  // Feature generation
  Perlin perlin(100);

  const int waterFeature = 0;
  const int landFeature = 1;
  const int forestFeature = 2;
  const int mountainFeature = 3;

  // initialize all features to water
  auto featureMap = std::vector<int>(mapWidth * mapHeight, waterFeature);

  // Place land and mountain features
  for (size_t i = 0; i < mapWidth * mapHeight; i++) {
    const float height = heightmap[i];

    // place land and mountains in 2x2 blocks
    if (validBlock(heightmap, i, 2.f, mapWidth, mapHeight)) {
      setBlock(featureMap, i, mountainFeature, mapWidth, mapHeight);
    } else if (validBlock(heightmap, i, 1.f, mapWidth, mapHeight)) {
      setBlock(featureMap, i, landFeature, mapWidth, mapHeight);
    }
  }

  // Map out forests into 2x2 blocks - skip every other y index
  for (size_t y = 0; y < mapHeight; y += 2) {
    for (size_t x = 0; x < mapWidth; x++) {
      const size_t index = x + y * mapWidth;
      if (!validBlock(featureMap, index, landFeature, mapWidth, mapHeight)) {
        continue;
      }

      const float xf0 = 10.f * (float(x) / float(mapWidth));
      const float yf0 = 10.f * (float(y) / float(mapWidth));
      const float xf1 = 10.f * (float(x + 1) / float(mapWidth));
      const float yf1 = 10.f * (float(y + 1) / float(mapWidth));

      const float s0 = perlin.Noise(xf0, yf0, 0.f);
      const float s1 = perlin.Noise(xf1, yf0, 0.f);
      const float s2 = perlin.Noise(xf0, yf1, 0.f);
      const float s3 = perlin.Noise(xf1, yf1, 0.f);

      if ((s0 + s1 + s2 + s3) / 4.f >= 0.6f) {
        setBlock(featureMap, index, forestFeature, mapWidth, mapHeight);
      }
    }
  }

  // Tile map creation from feature map
  entt::registry registry;
  auto world = registry.create();
  registry.emplace<WorldData>(world, mapWidth, mapHeight, mapDepth);

  auto& worldData = registry.get<WorldData>(world);

  const int waterTile = 801;
  const int grassTile = 81;
  const int forestTile = 326;
  const int mountainTile = 258;

  const uint8_t N = 0b0001;
  const uint8_t E = 0b0010;
  const uint8_t S = 0b0100;
  const uint8_t W = 0b1000;

  std::map<uint8_t, int> landsetMap;
  landsetMap.insert(std::pair(0, 258));
  landsetMap.insert(std::pair(N, 0));
  landsetMap.insert(std::pair(E, 0));
  landsetMap.insert(std::pair(S, 0));
  landsetMap.insert(std::pair(W, 0));
  landsetMap.insert(std::pair(N | E, 0));
  landsetMap.insert(std::pair(N | W, 0));
  landsetMap.insert(std::pair(N | W, 0));
  landsetMap.insert(std::pair(N, 0));
  landsetMap.insert(std::pair(N, 0));
  landsetMap.insert(std::pair(N, 0));
  landsetMap.insert(std::pair(N, 0));
  landsetMap.insert(std::pair(N, 0));
  landsetMap.insert(std::pair(N | S | E | W, 1));

  // for setting tiles, assign the full depth at once based on the feature
  size_t validTileCount = 0;
  for (size_t i = 0; i < mapWidth * mapHeight; i++) {
    const int x = i % mapWidth;
    const int y = i / mapWidth;

    if (featureMap[i] == waterFeature) {
      worldData.SetTileIndex(x, y, 0, waterTile);
      validTileCount++;
    } else if (featureMap[i] == landFeature) {
      worldData.SetTileIndex(x, y, 0, rng::next_int(1, 3));
      validTileCount++;
    } else if (featureMap[i] == mountainFeature) {
      worldData.SetTileIndex(x, y, 0, rng::next_int(1, 3));
      worldData.SetTileIndex(x, y, 1, mountainTile);
      validTileCount += 2;
    } else if (featureMap[i] == forestFeature) {
      worldData.SetTileIndex(x, y, 0, rng::next_int(1, 3));
      worldData.SetTileIndex(x, y, 1, rng::next_int(1, 3) == 1 ? 286 : 326);
      validTileCount += 2;
    } else {
      spdlog::critical("Invalid feature number ({}, {})", x, y);
    }
  }

  // world rendering setup
  auto tilesheet = TileSheet("content/textures/tileset.png", 16);
  //auto* buffer = new float[worldData.width * worldData.height * 24];
  auto* buffer = new float[validTileCount * 30];
  const auto tileSize = float(tilesheet.TileSize());
  const auto size = glm::vec2(1.f / tilesheet.Width(), 1.f / tilesheet.Height());

  size_t attributeCounter = 0;
  for (size_t i = 0; i < mapWidth * mapHeight * mapDepth; i++) {
    const auto tileIndex = worldData.GetTileIndex(i);
    if (tileIndex == -1) continue; // -1 index represents no tile

    const auto point = glm::vec2(
      float(tileIndex % tilesheet.Width()) / tilesheet.Width(),
      float(tileIndex / tilesheet.Width()) / tilesheet.Height()
    );

    // convert i to localized index (ignoring depth)
    const size_t localIndex = i % (mapWidth * mapHeight);
    const size_t x = localIndex % mapWidth;
    const size_t y = localIndex / mapWidth;
    const size_t z = i / (mapWidth * mapHeight);

    buffer[attributeCounter++] = x * tileSize;
    buffer[attributeCounter++] = y * tileSize;
    buffer[attributeCounter++] = z;
    buffer[attributeCounter++] = point.x;
    buffer[attributeCounter++] = point.y;

    buffer[attributeCounter++] = x * tileSize;
    buffer[attributeCounter++] = y * tileSize + tileSize;
    buffer[attributeCounter++] = z;
    buffer[attributeCounter++] = point.x;
    buffer[attributeCounter++] = point.y + size.y;

    buffer[attributeCounter++] = x * tileSize + tileSize;
    buffer[attributeCounter++] = y * tileSize;
    buffer[attributeCounter++] = z;
    buffer[attributeCounter++] = point.x + size.x;
    buffer[attributeCounter++] = point.y;


    buffer[attributeCounter++] = x * tileSize + tileSize;
    buffer[attributeCounter++] = y * tileSize;
    buffer[attributeCounter++] = z;
    buffer[attributeCounter++] = point.x + size.x;
    buffer[attributeCounter++] = point.y;

    buffer[attributeCounter++] = x * tileSize;
    buffer[attributeCounter++] = y * tileSize + tileSize;
    buffer[attributeCounter++] = z;
    buffer[attributeCounter++] = point.x;
    buffer[attributeCounter++] = point.y + size.y;

    buffer[attributeCounter++] = x * tileSize + tileSize;
    buffer[attributeCounter++] = y * tileSize + tileSize;
    buffer[attributeCounter++] = z;
    buffer[attributeCounter++] = point.x + size.x;
    buffer[attributeCounter++] = point.y + size.y;
  }

  VertexBuffer vBuffer;
  vBuffer.Bind();
  //vBuffer.SetBufferData(buffer, sizeof(float) * mapWidth * mapHeight * 24);
  vBuffer.SetBufferData(buffer, sizeof(float) * validTileCount * 30);
  vBuffer.EnableVertexAttribute(0);
  vBuffer.EnableVertexAttribute(1);
  vBuffer.VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)0);
  vBuffer.VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
  vBuffer.Unbind();

  delete[] buffer;

  auto shaderProgram = ShaderProgram(
      ReadTextFile("content/shaders/texturedVS.glsl"),
      ReadTextFile("content/shaders/texturedFS.glsl")
  );

  const float zoom = 1.f;
  glm::mat4 proj, view, model;
  proj = glm::ortho(0.f, float(window.Width()), float(window.Height()), 0.f, -10.f, 10.f);
  view = glm::scale(glm::mat4(1.f), glm::vec3(zoom));
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

  // fixed step logic
  float timer = window.Dt();
  const float fixed_step = 0.01667f; // 1/60th of a second

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

          cameraPos += (newPos - oldPos) / zoom;

          auto camera = glm::scale(glm::mat4(1.f), glm::vec3(zoom));
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
    /*ImGui::SliderFloat("Sea level", &waterLevel, 0.0f, 1.f);
    ImGui::SliderFloat("Mountain level", &mountainLevel, 0.0f, 1.f);
    if (ImGui::Button("Generate"))
      generateNewHeightmap(heightmapImage);*/
    ImGui::End();

    ImGui::Render();
    glClear(GL_COLOR_BUFFER_BIT);

    vBuffer.Bind();
    tilesheet.Bind();
    //glDrawArrays(GL_TRIANGLES, 0, mapWidth * mapHeight * 6);
    glDrawArrays(GL_TRIANGLES, 0, validTileCount * 6);
    vBuffer.Unbind();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    window.SwapBuffers();
  }

  gEventHandler.Post(ApplicationCloseEvent());
  spdlog::info("Program execution finished.");
  return 0;
}
