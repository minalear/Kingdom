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

using namespace worldgen;

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
    /*if (animTable.find(tileIndex) != animTable.end())
      tileIndex = animTable[tileIndex][animIndex];*/
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
  const int mapDepth = 5;

  const int seed = time(nullptr);

  auto heightmap = GenerateHeightmap(seed, mapWidth, mapHeight);
  for (auto& value : heightmap) {
    //value = (value <= seaLevel) ? 0.f : (value <= mountainLevel) ? 1.f : 2.f;
    value = (value <= 0.4f) ? 0.f : (value <= 0.55f) ? 1.f : (value <= 0.6f) ? 2.f : (value <= 0.65f) ? 3.f : 4.f;
  }

  //auto featureMap = std::vector<uint8_t>(mapWidth * mapHeight, waterFeature);
  auto featureMap = GenerateFeatureMap(seed, heightmap, mapWidth, mapHeight);

  // Tile map creation from feature map
  entt::registry registry;
  auto world = registry.create();
  registry.emplace<WorldData>(world, mapWidth, mapHeight, mapDepth);

  auto& worldData = registry.get<WorldData>(world);

  const int waterTile = 801;
  const int errorTile = 3;

  // Water animations
  std::map<int, std::array<int, 4>> waterAnim;

  waterAnim.insert(std::pair(420, std::array{ 420, 425, 430, 435 }));
  waterAnim.insert(std::pair(421, std::array{ 421, 426, 431, 436 }));
  waterAnim.insert(std::pair(422, std::array{ 422, 427, 432, 437 }));
  waterAnim.insert(std::pair(460, std::array{ 460, 465, 470, 475 }));
  waterAnim.insert(std::pair(461, std::array{ 461, 466, 471, 476 }));
  waterAnim.insert(std::pair(462, std::array{ 462, 467, 472, 477 }));
  waterAnim.insert(std::pair(500, std::array{ 500, 505, 510, 515 }));
  waterAnim.insert(std::pair(501, std::array{ 501, 506, 511, 516 }));
  waterAnim.insert(std::pair(502, std::array{ 502, 507, 512, 517 }));
  waterAnim.insert(std::pair(423, std::array{ 423, 428, 433, 438 }));
  waterAnim.insert(std::pair(424, std::array{ 424, 429, 434, 439 }));
  waterAnim.insert(std::pair(463, std::array{ 463, 468, 473, 478 }));
  waterAnim.insert(std::pair(464, std::array{ 464, 469, 474, 479 }));
  waterAnim.insert(std::pair(503, std::array{ 503, 508, 513, 518 }));
  waterAnim.insert(std::pair(504, std::array{ 504, 509, 514, 519 }));

  // Land bitmasks
  std::map<uint8_t, int> landsetMap;

  landsetMap.insert(std::pair(0, 418));
  landsetMap.insert(std::pair(N | S | E | W | NE | NW | SE | SW, 0));

  landsetMap.insert(std::pair(N, 900));
  landsetMap.insert(std::pair(S, 820));
  landsetMap.insert(std::pair(E, 780));
  landsetMap.insert(std::pair(W, 782));

  landsetMap.insert(std::pair(N | S, 860));
  landsetMap.insert(std::pair(E | W, 781));

  landsetMap.insert(std::pair(N | W | NW, 423));
  landsetMap.insert(std::pair(N | E | NE, 424));
  landsetMap.insert(std::pair(S | E | SE, 464));
  landsetMap.insert(std::pair(S | W | SW, 463));

  landsetMap.insert(std::pair(N | S | E | W, 940));
  landsetMap.insert(std::pair(N | E | W | NW, 821));
  landsetMap.insert(std::pair(N | E | W | NE, 822));
  landsetMap.insert(std::pair(S | E | W | SW, 861));
  landsetMap.insert(std::pair(S | E | W | SE, 862));
  landsetMap.insert(std::pair(N | S | E | NE, 901));
  landsetMap.insert(std::pair(N | S | E | NW, 902));
  landsetMap.insert(std::pair(N | S | W | SW, 941));
  landsetMap.insert(std::pair(N | S | E | SE, 942));

  landsetMap.insert(std::pair(E | W | S | SE | SW, 501));
  landsetMap.insert(std::pair(E | W | N | NE | NW, 421));
  landsetMap.insert(std::pair(N | S | E | NE | SE, 462));
  landsetMap.insert(std::pair(N | S | W | NW | SW, 460));

  landsetMap.insert(std::pair(N | S | E | W | NW | SE, 503));
  landsetMap.insert(std::pair(N | S | E | W | NE | SW, 504));

  landsetMap.insert(std::pair(N | S | E | W | NE | SE, 981));
  landsetMap.insert(std::pair(N | S | E | W | NW | SW, 982));
  landsetMap.insert(std::pair(N | S | E | W | NE | NW, 1021));
  landsetMap.insert(std::pair(N | S | E | W | SE | SW, 1022));

  landsetMap.insert(std::pair(N | S | E | W | NW | NE | SE, 422));
  landsetMap.insert(std::pair(N | S | E | W | NW | NE | SW, 420));
  landsetMap.insert(std::pair(N | S | E | W | NW | SW | SE, 500));
  landsetMap.insert(std::pair(N | S | E | W | NE | SE | SW, 502));

  // Tree bitmasks
  std::map<uint8_t, int> forestsetMap;

  forestsetMap.insert(std::pair(0, 286));
  forestsetMap.insert(std::pair(N | S | E | W | NE | NW | SE | SW, 321));

  forestsetMap.insert(std::pair(N, 243));
  forestsetMap.insert(std::pair(S, 163));
  forestsetMap.insert(std::pair(E, 164));
  forestsetMap.insert(std::pair(W, 166));

  forestsetMap.insert(std::pair(N | S, 203));
  forestsetMap.insert(std::pair(E | W, 165));

  forestsetMap.insert(std::pair(E | S, 204));
  forestsetMap.insert(std::pair(W | S, 205));
  forestsetMap.insert(std::pair(E | N, 244));
  forestsetMap.insert(std::pair(W | N, 245));

  forestsetMap.insert(std::pair(E | W | S, 206));
  forestsetMap.insert(std::pair(E | W | N, 246));
  forestsetMap.insert(std::pair(N | S | E, 207));
  forestsetMap.insert(std::pair(N | S | W, 247));
  forestsetMap.insert(std::pair(N | W | NW, 362));
  forestsetMap.insert(std::pair(N | E | NE, 360));
  forestsetMap.insert(std::pair(S | E | SE, 280));
  forestsetMap.insert(std::pair(S | W | SW, 282));

  forestsetMap.insert(std::pair(N | S | E | W, 125));
  forestsetMap.insert(std::pair(E | W | S | SE, 168));
  forestsetMap.insert(std::pair(E | W | S | SW, 169));
  forestsetMap.insert(std::pair(E | W | N | NE, 208));
  forestsetMap.insert(std::pair(E | W | N | NW, 209));
  forestsetMap.insert(std::pair(N | S | W | SW, 248));
  forestsetMap.insert(std::pair(N | S | E | SE, 249));
  forestsetMap.insert(std::pair(N | S | E | NE, 289));
  forestsetMap.insert(std::pair(N | S | W | NW, 288));

  forestsetMap.insert(std::pair(E | W | S | SE | SW, 281));
  forestsetMap.insert(std::pair(E | W | N | NE | NW, 361));
  forestsetMap.insert(std::pair(N | S | E | NE | SE, 320));
  forestsetMap.insert(std::pair(N | S | W | NW | SW, 322));

  forestsetMap.insert(std::pair(N | S | E | W | NW, 250));
  forestsetMap.insert(std::pair(N | S | E | W | NE, 251));
  forestsetMap.insert(std::pair(N | S | E | W | SE, 290));
  forestsetMap.insert(std::pair(N | S | E | W | SW, 291));

  forestsetMap.insert(std::pair(N | S | E | W | NE | NW, 170));
  forestsetMap.insert(std::pair(N | S | E | W | SE | SW, 171));
  forestsetMap.insert(std::pair(N | S | E | W | NW | SW, 210));
  forestsetMap.insert(std::pair(N | S | E | W | NE | SE, 211));
  forestsetMap.insert(std::pair(N | S | E | W | NW | SE, 364));
  forestsetMap.insert(std::pair(N | S | E | W | NE | SW, 363));

  forestsetMap.insert(std::pair(N | S | E | W | NW | NE | SE, 323));
  forestsetMap.insert(std::pair(N | S | E | W | NW | NE | SW, 324));
  forestsetMap.insert(std::pair(N | S | E | W | NW | SW | SE, 284));
  forestsetMap.insert(std::pair(N | S | E | W | NE | SE | SW, 283));

  // Mountain bitmasks
  std::map<uint8_t, int> mountainsetMap;
  mountainsetMap.insert(std::pair(0, 524));
  mountainsetMap.insert(std::pair(N | S | E | W | NW | NE | SW | SE, 441));

  mountainsetMap.insert(std::pair(N, 600));
  mountainsetMap.insert(std::pair(S, 520));
  mountainsetMap.insert(std::pair(E, 521));
  mountainsetMap.insert(std::pair(W, 523));

  mountainsetMap.insert(std::pair(N | S, 560));
  mountainsetMap.insert(std::pair(E | W, 522));
  mountainsetMap.insert(std::pair(S | E, 561));
  mountainsetMap.insert(std::pair(S | W, 562));
  mountainsetMap.insert(std::pair(N | E, 601));
  mountainsetMap.insert(std::pair(N | W, 602));

  mountainsetMap.insert(std::pair(S | E | SE, 400));
  mountainsetMap.insert(std::pair(S | W | SW, 402));
  mountainsetMap.insert(std::pair(N | E | NE, 480));
  mountainsetMap.insert(std::pair(N | W | NW, 482));
  mountainsetMap.insert(std::pair(N | E | W, 563));
  mountainsetMap.insert(std::pair(S | E | W, 564));
  mountainsetMap.insert(std::pair(N | S | E, 603));
  mountainsetMap.insert(std::pair(N | S | W, 604));

  mountainsetMap.insert(std::pair(N | E | W | NW, 485));
  mountainsetMap.insert(std::pair(N | E | W | NE, 486));
  mountainsetMap.insert(std::pair(S | E | W | SW, 525));
  mountainsetMap.insert(std::pair(S | E | W | SE, 526));

  mountainsetMap.insert(std::pair(N | S | E | W, 365));

  mountainsetMap.insert(std::pair(N | S | E | NE, 565));
  mountainsetMap.insert(std::pair(N | S | E | SE, 566));
  mountainsetMap.insert(std::pair(N | S | W | SW, 605));
  mountainsetMap.insert(std::pair(N | S | W | NE, 606));

  mountainsetMap.insert(std::pair(N | S | W | NE | NW | SE, 606));
  mountainsetMap.insert(std::pair(N | S | W | NE | NW | SW, 606));

  mountainsetMap.insert(std::pair(E | W | S | SW | SE, 401));
  mountainsetMap.insert(std::pair(E | W | N | NW | NE, 481));
  mountainsetMap.insert(std::pair(N | S | E | NE | SE, 440));
  mountainsetMap.insert(std::pair(N | S | W | NW | SW, 442));

  mountainsetMap.insert(std::pair(N | S | E | W | NE, 407));
  mountainsetMap.insert(std::pair(N | S | E | W | NW, 447));
  mountainsetMap.insert(std::pair(N | S | E | W | SW, 487));
  mountainsetMap.insert(std::pair(N | S | E | W | SE, 527));

  mountainsetMap.insert(std::pair(N | S | E | W | NE | SE, 405));
  mountainsetMap.insert(std::pair(N | S | E | W | NW | SW, 406));
  mountainsetMap.insert(std::pair(N | S | E | W | NE | NW, 445));
  mountainsetMap.insert(std::pair(N | S | E | W | SE | SW, 446));

  mountainsetMap.insert(std::pair(N | S | E | W | NE | SW, 483));
  mountainsetMap.insert(std::pair(N | S | E | W | NW | SE, 484));

  mountainsetMap.insert(std::pair(N | S | E | W | NE | SE | SW, 403));
  mountainsetMap.insert(std::pair(N | S | E | W | NW | SE | SW, 404));
  mountainsetMap.insert(std::pair(N | S | E | W | NE | NW | SE, 443));
  mountainsetMap.insert(std::pair(N | S | E | W | NE | NW | SW, 444));

  // for setting tiles, assign the full depth at once based on the feature
  size_t validTileCount = 0;
  for (size_t i = 0; i < mapWidth * mapHeight; i++) {
    const int x = i % mapWidth;
    const int y = i / mapWidth;

    // Set first layer tile to water
    worldData.SetTileIndex(x, y, 0, waterTile);
    validTileCount++;

    // Check if the tile is on land
    if ((featureMap[i] & landFeature) == landFeature) {
      const uint8_t bitmask = calculateBitmask(featureMap, i, landFeature, mapWidth);
      int index = (landsetMap.find(bitmask) == landsetMap.end()) ? errorTile : landsetMap[bitmask];

      // randomly vary between the base grass tiles
      if (index == 0) {
        index = rng::next_int(3); // 0 - 2 are base grass tiles
      }

      worldData.SetTileIndex(x, y, 1, index);
      validTileCount++;
    }

    // Check for mountain or forest features
    if ((featureMap[i] & mountainFeature) == mountainFeature) {
      const auto level1Bitmask = calculateBitmask(featureMap, i, level1Feature, mapWidth);
      const auto level2Bitmask = calculateBitmask(featureMap, i, level2Feature, mapWidth);
      const auto level3Bitmask = calculateBitmask(featureMap, i, level3Feature, mapWidth);

      // Level 1
      if ((featureMap[i] & level1Feature) == level1Feature) {
        const uint8_t bitmask = level3Bitmask | level2Bitmask | level1Bitmask;
        int index = (mountainsetMap.find(bitmask) == mountainsetMap.end()) ? errorTile : mountainsetMap[bitmask];
        worldData.SetTileIndex(x, y, 2, index);
        validTileCount++;
      }

      // Level 2
      if ((featureMap[i] & level2Feature) == level2Feature) {
        const uint8_t bitmask = level3Bitmask | level2Bitmask;
        int index = (mountainsetMap.find(bitmask) == mountainsetMap.end()) ? errorTile : mountainsetMap[bitmask];
        worldData.SetTileIndex(x, y, 3, index);
        validTileCount++;
      }

      // Level 3
      if ((featureMap[i] & level3Feature) == level3Feature) {
        const uint8_t bitmask = level3Bitmask;
        int index = (mountainsetMap.find(bitmask) == mountainsetMap.end()) ? errorTile : mountainsetMap[bitmask];
        worldData.SetTileIndex(x, y, 4, index);
        validTileCount++;
      }
    } else if ((featureMap[i] & forestFeature) == forestFeature) {
      const uint8_t bitmask = calculateBitmask(featureMap, i, forestFeature, mapWidth);
      int index = (forestsetMap.find(bitmask) == forestsetMap.end()) ? errorTile : forestsetMap[bitmask];

      // randomly vary between the base forest tiles
      if (index == 321) {
        const int rand = rng::next_int(0, 3);
        index = (rand == 0) ? 321 : (rand == 1) ? 285 : 325;
      }

      worldData.SetTileIndex(x, y, 2, index);
      validTileCount++;
    }
  }

  // Set tile flags
  for (size_t i = 0; i < mapWidth * mapHeight * mapDepth; i++) {
    if (waterAnim.find(worldData.tileData[i]) != waterAnim.end())
      worldData.tileFlags[i] = uint8_t(TILE_FLAGS::Anim);
  }

  // world rendering setup
  auto tilesheet = TileSheet("content/textures/tileset.png", 16);
  //auto* buffer = new float[worldData.width * worldData.height * 24];
  auto* buffer = new float[validTileCount * 30];

  generateBuffer(buffer, worldData, tilesheet, waterAnim, 0);

  VertexBuffer vBuffer;
  vBuffer.Bind();
  //vBuffer.SetBufferData(buffer, sizeof(float) * mapWidth * mapHeight * 24);
  vBuffer.SetBufferData(buffer, sizeof(float) * validTileCount * 30, GL_DYNAMIC_DRAW);
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
  ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.6f, 1.f);

  // Spritebatch and grid
  SpriteBatch sb(viewport_width, viewport_height);
  auto gridTexture = Texture2D("content/textures/grid_16x16.png");

  bool mouseDown = false;
  auto cameraPos = glm::vec2(0.f);
  auto oldPos = glm::vec2(0.f);
  auto newPos = glm::vec2(0.f);

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

        const int x = int((sdlEvent.motion.x - cameraPos.x) / 16.f);
        const int y = int((sdlEvent.motion.y - cameraPos.y) / 16.f);

        const int i = y * mapWidth + x;
        if (i >= 0 && i < mapWidth * mapHeight) {
          const uint8_t bitmask = calculateBitmask(featureMap, i, landFeature, mapWidth);
          auto dir = std::string();

          if ((bitmask & N) == N) dir += "N ";
          if ((bitmask & S) == S) dir += "S ";
          if ((bitmask & E) == E) dir += "E ";
          if ((bitmask & W) == W) dir += "W ";
          if ((bitmask & NE) == NE) dir += "NE ";
          if ((bitmask & NW) == NW) dir += "NW ";
          if ((bitmask & SE) == SE) dir += "SE ";
          if ((bitmask & SW) == SW) dir += "SW ";

          //spdlog::info("({}, {}) - forest mask: ({:b}) {}", x, y, bitmask, dir);
        }
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
      animTimer += fixed_step;
      if (animTimer >= 0.6f) {
        animIndex = (animIndex == 3) ? 0 : animIndex + 1;
        animTimer = 0.f;

        // Update buffer
        generateBuffer(buffer, worldData, tilesheet, waterAnim, animIndex);

        vBuffer.Bind();
        vBuffer.UpdateBufferData(buffer, 0, sizeof(float) * validTileCount * 30);
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
    glClear(GL_COLOR_BUFFER_BIT);

    shaderProgram.Use();
    vBuffer.Bind();
    tilesheet.Bind();
    //glDrawArrays(GL_TRIANGLES, 0, mapWidth * mapHeight * 6);
    glDrawArrays(GL_TRIANGLES, 0, validTileCount * 6);
    vBuffer.Unbind();

    sb.Begin(gridTexture);
    sb.Draw(gridTexture, cameraPos, glm::vec2(zoom));
    sb.End();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    window.SwapBuffers();
  }

  gEventHandler.Post(ApplicationCloseEvent());
  spdlog::info("Program execution finished.");
  return 0;
}
