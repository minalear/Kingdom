#ifndef KINGDOM_WORLD_GEN_DATA_H
#define KINGDOM_WORLD_GEN_DATA_H

#include <vector>

struct WorldGenData {
  int width, height;
  std::vector<uint8_t> heightmap, featuremap;

  WorldGenData(int width, int height);
};

#endif //KINGDOM_WORLD_GEN_DATA_H
