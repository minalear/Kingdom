#ifndef KINGDOM_BUILDING_DATA_H
#define KINGDOM_BUILDING_DATA_H

struct BuildingData {
  int spriteID;
  int x, y;

  BuildingData(int spriteID, int x, int y) :
    spriteID(spriteID), x(x), y(y) { }
};

#endif //KINGDOM_BUILDING_DATA_H
