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

  float seaLevel = 0.4f;
  float mountainLevel = 0.6f;

  auto heightmap = GenerateHeightmap(time(nullptr), mapWidth, mapHeight);
  for (auto& value : heightmap) {
    value = (value <= seaLevel) ? 0.f :
            (value <= mountainLevel) ? 1.f : 2.f;
  }

  // Feature generation
  Perlin perlin(100);

  entt::registry registry;
  auto world = registry.create();
  registry.emplace<WorldData>(world, mapWidth, mapHeight);

  auto& worldData = registry.get<WorldData>(world);

  for (size_t i = 0; i < mapWidth * mapHeight; i++) {
    const float height = heightmap[i];

    const int x = i % mapWidth;
    const int y = i / mapWidth;
    const float xf = float(x) / float(mapWidth);
    const float yf = float(y) / float(mapHeight);

    if (height == 0.f) worldData.SetTileIndex(i, 801);
    if (height == 1.f) {
      const float sample = perlin.Noise(0.5f * xf, 0.5f * yf, 0.f);
      const int id = (sample >= 0.75f) ? 326 : 81;
      worldData.SetTileIndex(i, id);
    }
    if (height == 2.f) worldData.SetTileIndex(i, 132);
  }

  /*const int waterTile = 0;
  const int mountainTile = 9;
  const int forestTile = 10;
  const int treeTile = 22;
  const int rockTile = 11;
  const int wheatTile = 20;
  const int waterRockTile = 44;

  // Landmass border generation
  for (size_t i = 0; i < mapWidth * mapHeight; i++) {
    if (worldData.GetTileIndex(i) == waterTile) continue; // ignore water
    const uint8_t bitmask = worldData.CalculateTileBitmask(i);
    worldData.SetTileIndex(i, tilesetIndices[bitmask]);
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
  }*/

  // world rendering setup
  auto tilesheet = TileSheet("content/textures/tileset.png", 16);
  auto* buffer = new float[worldData.width * worldData.height * 24];
  const auto tileSize = float(tilesheet.TileSize());
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

  delete[] buffer;

  auto shaderProgram = ShaderProgram(
      ReadTextFile("content/shaders/texturedVS.glsl"),
      ReadTextFile("content/shaders/texturedFS.glsl")
  );

  const float zoom = 0.5f;
  glm::mat4 proj, view, model;
  proj = glm::ortho(0.f, float(window.Width()), float(window.Height()), 0.f, -1.f, 1.f);
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

          cameraPos += newPos - oldPos;

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
    glDrawArrays(GL_TRIANGLES, 0, mapWidth * mapHeight * 6);
    vBuffer.Unbind();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    window.SwapBuffers();
  }

  gEventHandler.Post(ApplicationCloseEvent());
  spdlog::info("Program execution finished.");
  return 0;
}
