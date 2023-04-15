#ifndef KINGDOM_TILE_MAP_H
#define KINGDOM_TILE_MAP_H

#include <vector>
#include "vertex_buffer.h"
#include "tile_sheet.h"

struct TileMap {
  int mapWidth, mapHeight;
  int tileCount;

  VertexBuffer vertexBuffer;
  float* bufferData;

  TileMap(int mapWidth, int mapHeight, int tileCount);
  ~TileMap();

  void GenerateAnimatedBuffer(const std::vector<int>& tiles,
                              const std::vector<uint8_t>& tileData,
                              const TileSheet& tilesheet,
                              const AnimTable& animTable,
                              int animIndex);

  void GenerateBuffer(const std::vector<int>& tiles, const TileSheet& tilesheet);
};

#endif //KINGDOM_TILE_MAP_H
