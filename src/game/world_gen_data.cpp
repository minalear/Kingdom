#include "world_gen_data.h"

WorldGenData::WorldGenData(int width, int height)
  : width(width), height(height),
    heightmap(width * height, 0), featuremap(width * height, 0)
  { }