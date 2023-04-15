#ifndef KINGDOM_TILE_SHEET_H
#define KINGDOM_TILE_SHEET_H

#include <cstdint>
#include <map>
#include <array>

typedef std::map<int, std::array<int, 4>> AnimTable;
enum struct TILE_FLAGS {
  None = 0b00000000,
  Anim = 0b00000001
};

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
