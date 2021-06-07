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
};

#endif //KINGDOM_WORLDDATA_H
