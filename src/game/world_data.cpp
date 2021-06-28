#include "world_data.h"

WorldData::WorldData(const std::string& terrainPath, int width, int height, int depth)
  : width(width), height(height), depth(depth),
    tileData(width * height * depth, -1),
    tileFlags(width * height * depth, 0x00),
    terrainSet(terrainPath)
{
  animTable.insert(std::pair(420, std::array{ 420, 425, 430, 435 }));
  animTable.insert(std::pair(421, std::array{ 421, 426, 431, 436 }));
  animTable.insert(std::pair(422, std::array{ 422, 427, 432, 437 }));
  animTable.insert(std::pair(460, std::array{ 460, 465, 470, 475 }));
  animTable.insert(std::pair(461, std::array{ 461, 466, 471, 476 }));
  animTable.insert(std::pair(462, std::array{ 462, 467, 472, 477 }));
  animTable.insert(std::pair(500, std::array{ 500, 505, 510, 515 }));
  animTable.insert(std::pair(501, std::array{ 501, 506, 511, 516 }));
  animTable.insert(std::pair(502, std::array{ 502, 507, 512, 517 }));
  animTable.insert(std::pair(423, std::array{ 423, 428, 433, 438 }));
  animTable.insert(std::pair(424, std::array{ 424, 429, 434, 439 }));
  animTable.insert(std::pair(463, std::array{ 463, 468, 473, 478 }));
  animTable.insert(std::pair(464, std::array{ 464, 469, 474, 479 }));
  animTable.insert(std::pair(503, std::array{ 503, 508, 513, 518 }));
  animTable.insert(std::pair(504, std::array{ 504, 509, 514, 519 }));
}

int WorldData::GetTileIndex(int index) const {
  if (index < 0 || index >= width * height * depth) return -1;
  return tileData[index];
}
int WorldData::GetTileIndex(int x, int y, int z) const {
  if (x < 0 || x >= width || y < 0 || y >= height || z < 0 || z >= depth) return -1;
  return tileData[(x + y * width) + (z * width * height)];
}
void WorldData::SetTileIndex(int index, int tile) {
  if (index < 0 || index >= width * height * depth) return;
  tileData[index] = tile;
}
void WorldData::SetTileIndex(int x, int y, int z, int tile) {
  if (x < 0 || x >= width || y < 0 || y >= height || z < 0 || z >= depth) return;
  tileData[(x + y * width) + (z * width * height)] = tile;
}

uint8_t WorldData::CalculateTileBitmask(int index, int z) const {
  const int x = index % width;
  const int y = index / width;

  const int nw = GetTileIndex(x - 1, y - 1, z) == 0 ? 0 : 1;
  const int  n = GetTileIndex(x, y - 1, z)     == 0 ? 0 : 1;
  const int ne = GetTileIndex(x + 1, y - 1, z) == 0 ? 0 : 1;
  const int  w = GetTileIndex(x - 1, y, z)     == 0 ? 0 : 1;
  const int  e = GetTileIndex(x + 1, y, z)     == 0 ? 0 : 1;
  const int sw = GetTileIndex(x - 1, y + 1, z) == 0 ? 0 : 1;
  const int  s = GetTileIndex(x, y + 1, z)     == 0 ? 0 : 1;
  const int se = GetTileIndex(x + 1, y + 1, z) == 0 ? 0 : 1;

  uint8_t bitmask = 0x00;
  if (n && w) bitmask +=   1 * nw;
              bitmask +=   2 * n;
  if (n && e) bitmask +=   4 * ne;
              bitmask +=   8 * w;
              bitmask +=  16 * e;
  if (s && w) bitmask +=  32 * sw;
              bitmask +=  64 * s;
  if (s && e) bitmask += 128 * se;

  return bitmask;
}
