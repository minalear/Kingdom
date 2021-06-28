#ifndef KINGDOM_WORLDDATA_H
#define KINGDOM_WORLDDATA_H

#include <vector>
#include <map>
#include <array>
#include "terrain.h"

enum struct TILE_FLAGS {
  None = 0b00000000,
  Anim = 0b00000001
};

typedef std::map<int, std::array<int, 4>> AnimTable;

struct WorldData {
  int width;
  int height;
  int depth;

  Terrain terrainSet;
  std::map<int, std::array<int, 4>> animTable;

  std::vector<float> heightmap;
  std::vector<uint8_t> featuremap;

  std::vector<int> tileData;
  std::vector<uint8_t> tileFlags;
  size_t validTileCount;

  WorldData(const std::string& terrainPath, int width, int height, int depth);

  int GetTileIndex(int index) const;
  int GetTileIndex(int x, int y, int z) const;
  void SetTileIndex(int index, int tile);
  void SetTileIndex(int x, int y, int z, int tile);

  uint8_t CalculateTileBitmask(int index, int z) const;
};

#endif //KINGDOM_WORLDDATA_H
