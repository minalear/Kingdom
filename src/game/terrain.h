#ifndef KINGDOM_TERRAIN_H
#define KINGDOM_TERRAIN_H

#include <map>
#include <string>

class Terrain {
  std::map<std::string, std::map<uint8_t, int>> terrainData;

public:
  explicit Terrain(const std::string& path);

  int GetTileIndex(const std::string& terrain, uint8_t bitmask) const;
};

#endif //KINGDOM_TERRAIN_H
