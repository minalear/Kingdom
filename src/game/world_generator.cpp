#include "world_generator.h"
#include "../math/perlin.h"
#include "../math/func.h"
#include <vector>

const float WATER_LEVEL = 0.45f;

std::vector<float> GenerateHeightmap(const uint32_t seed, const int mapWidth, const int mapHeight) {
  const size_t nTiles = mapWidth * mapHeight;
  std::vector<float> heightmap(nTiles, 0.f);

  // create basic circle
  const int centerX = mapWidth / 2;
  const int centerY = mapHeight / 2;
  const float maxDistance = dist(0, 0, centerX, centerY);

  for (size_t i = 0; i < nTiles; i++) {
    const int x = i % mapWidth;
    const int y = i / mapWidth;

    float elevation = 1.f - (dist(x, y, centerX, centerY) / maxDistance);
    if (elevation <= WATER_LEVEL) elevation = 0.f;

    heightmap[i] = elevation;
  }

  // perlin noise generator
  Perlin perlin(seed);

  // layer noise to vary elevation
  for (size_t i = 0; i < nTiles; i++) {
    // bounded coordinates between 0.f and 1.f
    const float x = float(i % mapWidth) / float(mapWidth);
    const float y = float(i / mapWidth) / float(mapHeight);

    const float n1 = heightmap[i];
    const float n2 = perlin.Noise(x, y, 0.f);
    const float n3 = perlin.Noise(5.f * x, 5.f * y, 0.f);
    const float n4 = perlin.Noise(10.f * x, 10.f * y, 0.f);

    float elevation = n1 * (n2 + n3 + n4) / 3.f;
    if (elevation <= 0.3f) elevation = 0.f;

    heightmap[i] = elevation;
  }

  return heightmap;
}