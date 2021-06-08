#ifndef KINGDOM_TILE_SHEET_H
#define KINGDOM_TILE_SHEET_H

#include <cstdint>

class TileSheet {
  uint32_t id;
  int tileSize;
  int width;
  int height;
  int textureWidth;
  int textureHeight;

public:
  TileSheet(const char* path, int tileSize);
  ~TileSheet();

  int TileSize() const;
  int TextureWidth() const;
  int TextureHeight() const;
  int Width() const;
  int Height() const;

  void Bind() const;
};

#endif //KINGDOM_TILE_SHEET_H
