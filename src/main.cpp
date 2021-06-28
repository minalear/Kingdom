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
#include "game/terrain.h"

void generateMinimapTexture(const Texture2D& minimap, const std::vector<uint8_t>& featuremap) {
  using namespace worldgen;
  auto imageData = new uint8_t[featuremap.size() * 4];

  size_t i = 0;
  for (auto feature : featuremap) {
    if (feature == waterFeature) {
      imageData[i * 4 + 0] = 0;
      imageData[i * 4 + 1] = 0;
      imageData[i * 4 + 2] = 255;
      imageData[i * 4 + 3] = 255;
    } else if ((feature & mountainFeature) == mountainFeature) {
      imageData[i * 4 + 0] = 255;
      imageData[i * 4 + 1] = 0;
      imageData[i * 4 + 2] = 0;
      imageData[i * 4 + 3] = 255;
    } else if ((feature & landFeature) == landFeature){
      imageData[i * 4 + 0] = 0;
      imageData[i * 4 + 1] = 255;
      imageData[i * 4 + 2] = 0;
      imageData[i * 4 + 3] = 255;
    }

    if ((feature & borderFlag) == borderFlag) {
      imageData[i * 4 + 0] = 255;
      imageData[i * 4 + 1] = 125;
      imageData[i * 4 + 2] = 125;
      imageData[i * 4 + 3] = 255;
    }

    i++;
  }

  minimap.SetTextureData(imageData);
  delete[] imageData;
}

void generateBuffer(float* buffer, WorldData& worldData, TileSheet& tilesheet, std::map<int, std::array<int, 4>> animTable, int animIndex) {
  const int mapWidth = worldData.width;
  const int mapHeight = worldData.height;
  const int mapDepth = worldData.depth;

  const auto tileSize = float(tilesheet.TileSize());
  const auto sheetSize = glm::vec2(1.f / tilesheet.Width(), 1.f / tilesheet.Height());

  size_t attributeCounter = 0;
  for (size_t i = 0; i < mapWidth * mapHeight * mapDepth; i++) {
    auto tileIndex = worldData.GetTileIndex(i);
    if (tileIndex == -1) continue; // -1 index represents no tile

    // Map lookup turned out to be too slow, setting a flag for animation is much quicker
    if ((worldData.tileFlags[i] & uint8_t(TILE_FLAGS::Anim)) == uint8_t(TILE_FLAGS::Anim))
      tileIndex = animTable[tileIndex][animIndex];

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
    buffer[attributeCounter++] = point.y + sheetSize.y;

    buffer[attributeCounter++] = x * tileSize + tileSize;
    buffer[attributeCounter++] = y * tileSize;
    buffer[attributeCounter++] = z;
    buffer[attributeCounter++] = point.x + sheetSize.x;
    buffer[attributeCounter++] = point.y;


    buffer[attributeCounter++] = x * tileSize + tileSize;
    buffer[attributeCounter++] = y * tileSize;
    buffer[attributeCounter++] = z;
    buffer[attributeCounter++] = point.x + sheetSize.x;
    buffer[attributeCounter++] = point.y;

    buffer[attributeCounter++] = x * tileSize;
    buffer[attributeCounter++] = y * tileSize + tileSize;
    buffer[attributeCounter++] = z;
    buffer[attributeCounter++] = point.x;
    buffer[attributeCounter++] = point.y + sheetSize.y;

    buffer[attributeCounter++] = x * tileSize + tileSize;
    buffer[attributeCounter++] = y * tileSize + tileSize;
    buffer[attributeCounter++] = z;
    buffer[attributeCounter++] = point.x + sheetSize.x;
    buffer[attributeCounter++] = point.y + sheetSize.y;
  }
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

  //const int seed = time(nullptr);
  const int seed = 8008135;

  // Entity/Component
  entt::registry ecsRegister;
  auto worldEntity = ecsRegister.create();
  ecsRegister.emplace<WorldData>(worldEntity, worldgen::GenerateGameWorld("content/data/tileset.terrain", seed, mapWidth, mapHeight));

  auto& worldData = ecsRegister.get<WorldData>(worldEntity);

  // minimap texture generation
  auto minimapTexture = Texture2D(mapWidth, mapHeight);
  generateMinimapTexture(minimapTexture, worldData.featuremap);

  // world rendering setup
  auto tilesheet = TileSheet("content/textures/tileset.png", 16);
  auto* buffer = new float[worldData.validTileCount * 30];

  generateBuffer(buffer, worldData, tilesheet, worldData.animTable, 0);

  VertexBuffer vBuffer;
  vBuffer.Bind();
  vBuffer.SetBufferData(buffer, sizeof(float) * worldData.validTileCount * 30, GL_DYNAMIC_DRAW);
  vBuffer.EnableVertexAttribute(0);
  vBuffer.EnableVertexAttribute(1);
  vBuffer.VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)0);
  vBuffer.VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
  vBuffer.Unbind();

  //delete[] buffer;

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

  // Spritebatch and grid
  SpriteBatch sb(viewport_width, viewport_height);
  auto gridTexture = Texture2D("content/textures/grid_16x16.png");

  bool mouseDown = false;
  auto cameraPos = glm::vec2(0.f);
  auto oldPos = glm::vec2(0.f);
  auto newPos = glm::vec2(0.f);

  auto camera = glm::mat4(1.f);

  // fixed step logic
  float timer = window.Dt();
  float animTimer = 0.f;
  int animIndex = 0;
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
      } else if (sdlEvent.type == SDL_KEYDOWN) {
        gEventHandler.Post(KeyboardEvent(EventType::KeyboardButtonDown, sdlEvent.key.keysym.scancode));
      }

      if (sdlEvent.type == SDL_MOUSEBUTTONDOWN && sdlEvent.button.button == SDL_BUTTON_LEFT) {
        mouseDown = true;
        oldPos = newPos = glm::vec2(sdlEvent.button.x, sdlEvent.button.y);
      } else if (sdlEvent.type == SDL_MOUSEBUTTONUP && sdlEvent.button.button == SDL_BUTTON_LEFT) {
        mouseDown = false;

        auto worldPos = glm::inverse(camera) * glm::vec4(sdlEvent.button.x, sdlEvent.button.y, 0.f, 1.f);
        const int x = int(worldPos.x / tilesheet.TileSize());
        const int y = int(worldPos.y / tilesheet.TileSize());
        const int i = x + y * mapWidth;

        //spdlog::info("tile: ({}, {})", x, y);
      }
    }

    if (sdlEvent.type == SDL_MOUSEMOTION) {
      gEventHandler.Post(MouseMoveEvent(sdlEvent.motion.x, sdlEvent.motion.y));

      if (mouseDown) {
        oldPos = newPos;
        newPos = glm::vec2(sdlEvent.motion.x, sdlEvent.motion.y);

        cameraPos += (newPos - oldPos) / zoom;

        camera = glm::scale(glm::mat4(1.f), glm::vec3(zoom));
        camera = glm::translate(camera, glm::vec3(cameraPos, 0.f));
        shaderProgram.Use();
        shaderProgram.SetUniform("view", camera);
      }
    }

    // Update logic
    timer += window.Dt();
    if (timer >= fixed_step) {
      timer -= fixed_step;

      // do fixed-step logic here with fixed_step as dt
      animTimer += fixed_step;
      if (animTimer >= 0.45f) {
        animIndex = (animIndex == 3) ? 0 : animIndex + 1;
        animTimer = 0.f;

        // Update buffer
        generateBuffer(buffer, worldData, tilesheet, worldData.animTable, animIndex);

        vBuffer.Bind();
        vBuffer.UpdateBufferData(buffer, 0, sizeof(float) * worldData.validTileCount * 30);
        vBuffer.Unbind();

        //spdlog::info("ANIM INDEX: {}", animIndex);
      }
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
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    shaderProgram.Use();
    vBuffer.Bind();
    tilesheet.Bind();
    //glDrawArrays(GL_TRIANGLES, 0, mapWidth * mapHeight * 6);
    glDrawArrays(GL_TRIANGLES, 0, worldData.validTileCount * 6);
    vBuffer.Unbind();

    sb.Begin(gridTexture);
    sb.GetShaderProgram().SetUniform("view", camera);
    sb.Draw(gridTexture, glm::vec2(0.f), glm::vec2(1.f));
    sb.GetShaderProgram().SetUniform("view", glm::mat4(1.f));
    sb.End();

    sb.Begin(minimapTexture);
    sb.Draw(minimapTexture, glm::vec2(viewport_width - minimapTexture.Width() - 10.f, 10.f), glm::vec2(1.f));
    sb.End();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    window.SwapBuffers();
  }

  gEventHandler.Post(ApplicationCloseEvent());
  spdlog::info("Program execution finished.");
  return 0;
}
