#ifndef KINGDOM_WORLDDATA_H
#define KINGDOM_WORLDDATA_H

struct WorldData {
  int width;
  int height;

  int* tileData;

  WorldData(int width, int height) : width(width), height(height) {
    tileData = new int[width * height];
  }
  ~WorldData() {
    delete[] tileData;
  }

  int GetTileIndex(int index) const {
    if (index < 0 || index >= width * height) return 0;
    return tileData[index];
  }
  int GetTileIndex(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= width) return 0;
    return tileData[x + y * width];
  }

  void SetTileIndex(int index, int tile) {
    if (index < 0 || index >= width * height) return;
    tileData[index] = tile;
  }

  void SetTileIndex(int x, int y, int tile) {
    if (x < 0 || x >= width || y < 0 || y >= width) return;
    tileData[x + y * width] = tile;
  }
};

#endif //KINGDOM_WORLDDATA_H
