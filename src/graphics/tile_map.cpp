#include "tile_map.h"
#include "glm/glm.hpp"

const int AttributeCount = 30;

TileMap::TileMap(int mapWidth, int mapHeight, int tileCount)
  : mapWidth(mapWidth), mapHeight(mapHeight), tileCount(tileCount)
{
  bufferData = new float[tileCount * AttributeCount];

  vertexBuffer.Bind();
  vertexBuffer.SetBufferData(bufferData, sizeof(float) * tileCount * AttributeCount, GL_DYNAMIC_DRAW);
  vertexBuffer.EnableVertexAttribute(0);
  vertexBuffer.EnableVertexAttribute(1);
  vertexBuffer.VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)0);
  vertexBuffer.VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
  vertexBuffer.Unbind();
}
TileMap::~TileMap() {
  delete[] bufferData;
}

void TileMap::GenerateAnimatedBuffer(const std::vector<int>& tiles,
                             const std::vector<uint8_t>& tileData,
                             const TileSheet& tilesheet,
                             const AnimTable& animTable,
                             int animIndex) {
  const auto tileSize = float(tilesheet.TileSize());
  const auto sheetSize = glm::vec2(1.f / tilesheet.Width(), 1.f / tilesheet.Height());

  size_t attributeCounter = 0;
  for (size_t i = 0; i < tiles.size(); i++) {
    auto tileIndex = tiles[i];
    if (tileIndex == -1) continue; // -1 index represents no tile

    if ((tileData[i] & uint8_t(TILE_FLAGS::Anim)) == uint8_t(TILE_FLAGS::Anim))
      tileIndex = animTable.at(tileIndex)[animIndex];

    const auto point = glm::vec2(
      float(tileIndex % tilesheet.Width()) / tilesheet.Width(),
      float(tileIndex / tilesheet.Width()) / tilesheet.Height()
    );

    // convert i to localized index (ignoring depth)
    const size_t localIndex = i % (mapWidth * mapHeight);
    const size_t x = localIndex % mapWidth;
    const size_t y = localIndex / mapWidth;
    const size_t z = i / (mapWidth * mapHeight);

    bufferData[attributeCounter++] = x * tileSize;
    bufferData[attributeCounter++] = y * tileSize;
    bufferData[attributeCounter++] = z;
    bufferData[attributeCounter++] = point.x;
    bufferData[attributeCounter++] = point.y;

    bufferData[attributeCounter++] = x * tileSize;
    bufferData[attributeCounter++] = y * tileSize + tileSize;
    bufferData[attributeCounter++] = z;
    bufferData[attributeCounter++] = point.x;
    bufferData[attributeCounter++] = point.y + sheetSize.y;

    bufferData[attributeCounter++] = x * tileSize + tileSize;
    bufferData[attributeCounter++] = y * tileSize;
    bufferData[attributeCounter++] = z;
    bufferData[attributeCounter++] = point.x + sheetSize.x;
    bufferData[attributeCounter++] = point.y;


    bufferData[attributeCounter++] = x * tileSize + tileSize;
    bufferData[attributeCounter++] = y * tileSize;
    bufferData[attributeCounter++] = z;
    bufferData[attributeCounter++] = point.x + sheetSize.x;
    bufferData[attributeCounter++] = point.y;

    bufferData[attributeCounter++] = x * tileSize;
    bufferData[attributeCounter++] = y * tileSize + tileSize;
    bufferData[attributeCounter++] = z;
    bufferData[attributeCounter++] = point.x;
    bufferData[attributeCounter++] = point.y + sheetSize.y;

    bufferData[attributeCounter++] = x * tileSize + tileSize;
    bufferData[attributeCounter++] = y * tileSize + tileSize;
    bufferData[attributeCounter++] = z;
    bufferData[attributeCounter++] = point.x + sheetSize.x;
    bufferData[attributeCounter++] = point.y + sheetSize.y;
  }

  vertexBuffer.Bind();
  vertexBuffer.UpdateBufferData(bufferData, 0, sizeof(float) * tileCount * 30);
  vertexBuffer.Unbind();
}

void TileMap::GenerateBuffer(const std::vector<int> &tiles, const TileSheet &tilesheet) {
  const auto tileSize = float(tilesheet.TileSize());
  const auto sheetSize = glm::vec2(1.f / tilesheet.Width(), 1.f / tilesheet.Height());

  size_t attributeCounter = 0;
  for (size_t i = 0; i < tiles.size(); i++) {
    auto tileIndex = tiles[i];
    if (tileIndex == -1) continue; // -1 index represents no tile

    const auto point = glm::vec2(
        float(tileIndex % tilesheet.Width()) / tilesheet.Width(),
        float(tileIndex / tilesheet.Width()) / tilesheet.Height()
    );

    // convert i to localized index (ignoring depth)
    const size_t localIndex = i % (mapWidth * mapHeight);
    const size_t x = localIndex % mapWidth;
    const size_t y = localIndex / mapWidth;
    const size_t z = i / (mapWidth * mapHeight);

    bufferData[attributeCounter++] = x * tileSize;
    bufferData[attributeCounter++] = y * tileSize;
    bufferData[attributeCounter++] = z;
    bufferData[attributeCounter++] = point.x;
    bufferData[attributeCounter++] = point.y;

    bufferData[attributeCounter++] = x * tileSize;
    bufferData[attributeCounter++] = y * tileSize + tileSize;
    bufferData[attributeCounter++] = z;
    bufferData[attributeCounter++] = point.x;
    bufferData[attributeCounter++] = point.y + sheetSize.y;

    bufferData[attributeCounter++] = x * tileSize + tileSize;
    bufferData[attributeCounter++] = y * tileSize;
    bufferData[attributeCounter++] = z;
    bufferData[attributeCounter++] = point.x + sheetSize.x;
    bufferData[attributeCounter++] = point.y;


    bufferData[attributeCounter++] = x * tileSize + tileSize;
    bufferData[attributeCounter++] = y * tileSize;
    bufferData[attributeCounter++] = z;
    bufferData[attributeCounter++] = point.x + sheetSize.x;
    bufferData[attributeCounter++] = point.y;

    bufferData[attributeCounter++] = x * tileSize;
    bufferData[attributeCounter++] = y * tileSize + tileSize;
    bufferData[attributeCounter++] = z;
    bufferData[attributeCounter++] = point.x;
    bufferData[attributeCounter++] = point.y + sheetSize.y;

    bufferData[attributeCounter++] = x * tileSize + tileSize;
    bufferData[attributeCounter++] = y * tileSize + tileSize;
    bufferData[attributeCounter++] = z;
    bufferData[attributeCounter++] = point.x + sheetSize.x;
    bufferData[attributeCounter++] = point.y + sheetSize.y;
  }

  vertexBuffer.Bind();
  vertexBuffer.UpdateBufferData(bufferData, 0, sizeof(float) * tileCount * 30);
  vertexBuffer.Unbind();
}
