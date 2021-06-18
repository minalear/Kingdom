#ifndef KINGDOM_WORLDDATA_H
#define KINGDOM_WORLDDATA_H

#include <vector>

struct WorldData {
  int width;
  int height;
  int depth;

  std::vector<int> tileData;

  WorldData(int width, int height, int depth);

  int GetTileIndex(int index) const;
  int GetTileIndex(int x, int y, int z) const;
  void SetTileIndex(int index, int tile);
  void SetTileIndex(int x, int y, int z, int tile);

  uint8_t CalculateTileBitmask(int index, int z) const;
};

#endif //KINGDOM_WORLDDATA_H
