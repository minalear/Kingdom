#include "world_data.h"

WorldData::WorldData(int width, int height, int depth)
  : width(width), height(height), depth(depth),
    tileData(width * height * depth, -1),
    tileFlags(width * height * depth, 0x00)
{ }

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
