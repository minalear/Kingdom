#include "world_generator.h"
#include "../math/perlin.h"
#include "../math/func.h"
#include "spdlog/spdlog.h"
#include <vector>

using namespace worldgen;

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
      if (bitmask != 0b11111111) {
        featureMap[i] = featureMap[i] | borderFlag;
      }
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