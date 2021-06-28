#include "world_generator.h"
#include "../math/perlin.h"
#include "../math/func.h"
#include "../math/rng.h"
#include "spdlog/spdlog.h"
#include <vector>

using namespace worldgen;

const int waterTile = 801;

WorldData worldgen::GenerateGameWorld(const std::string& terrainPath, uint32_t seed, int mapWidth, int mapHeight) {
  WorldData worldData(terrainPath, mapWidth, mapHeight, 5);

  // Get generated heightmap and squash values
  worldData.heightmap = GenerateHeightmap(seed, mapWidth, mapHeight);
  for (auto& value : worldData.heightmap) {
    value = (value <= 0.4f) ? 0.f : (value <= 0.55f) ? 1.f : (value <= 0.6f) ? 2.f : (value <= 0.65f) ? 3.f : 4.f;
  }

  // Generate features based off heightmap
  worldData.featuremap = GenerateFeatureMap(seed, worldData.heightmap, mapWidth, mapHeight);
  CreateTileMap(worldData);

  return worldData;
}

std::vector<float> worldgen::GenerateHeightmap(const uint32_t seed, const int mapWidth, const int mapHeight) {
  const size_t nTiles = mapWidth * mapHeight;
  std::vector<float> heightmap(nTiles, 0.f);

  // create basic circle
  const int centerX = mapWidth / 2;
  const int centerY = mapHeight / 2;
  const float maxDistance = dist(0, 0, centerX, centerY);

  for (size_t i = 0; i < nTiles; i++) {
    const int x = i % mapWidth;
    const int y = i / mapWidth;

    heightmap[i] = 1.f - (dist(x, y, centerX, centerY) / maxDistance);
  }

  // perlin noise generator
  Perlin perlin(seed);

  float highest = -1.f;

  // layer noise to vary elevation
  for (size_t i = 0; i < nTiles; i++) {
    // bounded coordinates between 0.f and 1.f
    const float x = float(i % mapWidth) / float(mapWidth);
    const float y = float(i / mapWidth) / float(mapHeight);

    const float n1 = heightmap[i];
    const float n2 = 1.3f * perlin.Noise(x, y, 0.f);
    const float n3 = 1.3f * perlin.Noise(5.f * x, 5.f * y, 0.f);
    const float n4 = 1.3f * perlin.Noise(10.f * x, 10.f * y, 0.f);
    const float n5 = 1.3f * perlin.Noise(20.f * x, 20.f * y, 0.f);

    float elevation = n1 * ((n2 + n3 + n4 + n5) / 4.f);
    if (elevation > 1.f) elevation = 1.f; // bounds checking
    if (elevation < 0.f) elevation = 0.f;
    if (elevation > highest) highest = elevation;

    heightmap[i] = elevation;
  }

  // spdlog::info("Highest Point: {}", highest);

  return heightmap;
}

std::vector<uint8_t> worldgen::GenerateFeatureMap(uint32_t seed, std::vector<float> heightmap, int mapWidth, int mapHeight) {
  auto featureMap = std::vector<uint8_t>(mapWidth * mapHeight, waterFeature);

  // First generate landmass and determine land borders, then we place mountains and trees, avoiding the borders
  for (size_t i = 0; i < mapWidth * mapHeight; i++) {
    if (heightmap[i] == 0.f) featureMap[i] = waterFeature;
    if (heightmap[i] == 1.f) featureMap[i] = landFeature;
  }

  // Determine land borders by calculating bitmasks and adding a flag to indicate borders
  for (size_t i = 0; i < mapWidth * mapHeight; i++) {
    if ((featureMap[i] & landFeature) == landFeature) {
      const int bitmask = calculateBitmask(featureMap, i, landFeature, mapWidth);
      if (bitmask != 0b11111111) featureMap[i] |= borderFlag;
    }
  }

  // Place mountains on land and not on borders
  for (size_t i = 0; i < mapWidth * mapHeight; i++) {
    if (heightmap[i] >= 2.f && (featureMap[i] & borderFlag) != borderFlag) {
      if (heightmap[i] == 2.f) featureMap[i] = level1Feature;
      if (heightmap[i] == 3.f) featureMap[i] = level2Feature;
      if (heightmap[i] == 4.f) featureMap[i] = level3Feature;
    }
  }

  // Determine mountain borders by calculating bitmasks and adding a flag
  for (size_t i = 0; i < mapWidth * mapHeight; i++) {
    if ((featureMap[i] & mountainFeature) == mountainFeature) {
      const int bitmask = calculateBitmask(featureMap, i, mountainFeature, mapWidth);
      if (bitmask != 0b11111111) featureMap[i] |= borderFlag;
    }
  }

  Perlin perlin(seed);

  for (size_t i = 0; i < mapWidth * mapHeight; i++) {
    if (featureMap[i] == landFeature) {
      const size_t x = i % mapWidth;
      const size_t y = i / mapWidth;

      const float xf = float(x) / float(mapWidth);
      const float yf = float(y) / float(mapHeight);

      /*const float s0 = perlin.Noise(xf, yf, 0.f);
      const float s1 = perlin.Noise(50.f * xf, 50.f * yf, 2.f);
      const float s2 = perlin.Noise(200.f * xf, 200.f * yf, 4.f);

      if (featureMap[i] == landFeature && (s0 + s1 + s2) / 3.f >= 0.55f) {
        featureMap[i] = forestFeature;
      }*/

      const float s0 = perlin.Noise(0.5f * xf, 0.5f * yf, 0.f);
      const float s1 = perlin.Noise(200.f * xf, 200.f * yf, 2.f);

      if (s0 >= 0.6f || s1 >= 0.5f) {
        featureMap[i] = forestFeature;
      }
    }
  }

  return featureMap;
}

void worldgen::CreateTileMap(WorldData &worldData) {
  const int mapWidth = worldData.width;
  const int mapHeight = worldData.height;
  const int mapDepth = worldData.depth;

  size_t validTileCount = 0;
  for (size_t i = 0; i < mapWidth * mapHeight; i++) {
    const int x = int(i % mapWidth);
    const int y = int(i / mapWidth);

    // Set first layer tile to water
    worldData.SetTileIndex(x, y, 0, waterTile);
    validTileCount++;

    // Check if the tile is on land
    if ((worldData.featuremap[i] & landFeature) == landFeature) {
      const uint8_t bitmask = calculateBitmask(worldData.featuremap, i, landFeature, mapWidth);
      int index = worldData.terrainSet.GetTileIndex("land", bitmask);

      // randomly vary between the base grass tiles
      if (index == 0) {
        index = rng::next_int(3); // 0 - 2 are base grass tiles
      }

      worldData.SetTileIndex(x, y, 1, index);
      validTileCount++;
    }

    // Check for mountain or forest features
    if ((worldData.featuremap[i] & mountainFeature) == mountainFeature) {
      const auto level1Bitmask = calculateBitmask(worldData.featuremap, i, level1Feature, mapWidth);
      const auto level2Bitmask = calculateBitmask(worldData.featuremap, i, level2Feature, mapWidth);
      const auto level3Bitmask = calculateBitmask(worldData.featuremap, i, level3Feature, mapWidth);

      // Determine elevation based on feature flags
      if ((worldData.featuremap[i] & level1Feature) == level1Feature) {
        const uint8_t bitmask = level3Bitmask | level2Bitmask | level1Bitmask;

        int index = worldData.terrainSet.GetTileIndex("mountain", bitmask);
        worldData.SetTileIndex(x, y, 2, index);
        validTileCount++;
      }

      if ((worldData.featuremap[i] & level2Feature) == level2Feature) {
        const uint8_t bitmask = level3Bitmask | level2Bitmask;

        int index = worldData.terrainSet.GetTileIndex("mountain", bitmask);
        worldData.SetTileIndex(x, y, 3, index);
        validTileCount++;
      }

      if ((worldData.featuremap[i] & level3Feature) == level3Feature) {
        const uint8_t bitmask =  level3Bitmask;

        int index = worldData.terrainSet.GetTileIndex("mountain", bitmask);
        worldData.SetTileIndex(x, y, 4, index);
        validTileCount++;
      }
    } else if ((worldData.featuremap[i] & forestFeature) == forestFeature) {
      const uint8_t bitmask = calculateBitmask(worldData.featuremap, i, forestFeature, mapWidth);
      int index = worldData.terrainSet.GetTileIndex("forest", bitmask);

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
    if (worldData.animTable.find(worldData.tileData[i]) != worldData.animTable.end())
      worldData.tileFlags[i] = uint8_t(TILE_FLAGS::Anim);
  }

  worldData.validTileCount = validTileCount;
}
