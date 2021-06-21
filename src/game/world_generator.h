#ifndef KINGDOM_WORLD_GENERATOR_H
#define KINGDOM_WORLD_GENERATOR_H

#include <cstdint>
#include <vector>

namespace worldgen {
  const uint8_t waterFeature    = 0b00000000;
  const uint8_t landFeature     = 0b00000001;
  const uint8_t borderFlag      = 0b00000010;
  const uint8_t forestFeature   = 0b00000100 | landFeature;
  const uint8_t mountainFeature = 0b00001000 | landFeature;
  const uint8_t level1Feature   = 0b00010000 | mountainFeature;
  const uint8_t level2Feature   = 0b00100000 | level1Feature;
  const uint8_t level3Feature   = 0b01000000 | level2Feature;

  // Coordinate bitmasks
  const uint8_t NW = 0b00000001;
  const uint8_t  N = 0b00000010;
  const uint8_t NE = 0b00000100;
  const uint8_t  W = 0b00001000;
  const uint8_t  E = 0b00010000;
  const uint8_t SW = 0b00100000;
  const uint8_t  S = 0b01000000;
  const uint8_t SE = 0b10000000;

  template <typename T>
  bool isUniformBlock(const std::vector<T>& vec, size_t i0, T value, int mapWidth) {
    // get indices of the 2x2 block
    const size_t i1 = i0 + 1;
    const size_t i2 = i0 + mapWidth;
    const size_t i3 = i2 + 1;

    const size_t len = vec.size();
    if (i0 >= len || i1 >= len || i2 >= len || i3 >= len) return false;
    if (i0 < 0 || i1 <= 0 || i2 <= 0 || i3 <= 0) return false;

    return (vec[i0] == value && vec[i1] == value && vec[i2] == value && vec[i3] == value);
  }

  template <typename T>
  void setBlock(std::vector<T>& vec, size_t i0, T value, int mapWidth) {
    // get indices of the 2x2 block
    const size_t i1 = i0 + 1;
    const size_t i2 = i0 + mapWidth;
    const size_t i3 = i2 + 1;

    vec[i0] = vec[i1] = vec[i2] = vec[i3] = value;
  }

  template <typename T>
  T getValueByCoord(const std::vector<T>& vec, int x, int y, int mapWidth) {
    const size_t index = x + y * mapWidth;
    if (index < 0 || index >= vec.size()) return -1;
    return vec[index];
  }

  template <typename T>
  uint8_t calculateBitmask(const std::vector<T>& vec, size_t index, int flag, int mapWidth) {
    const int x = index % mapWidth;
    const int y = index / mapWidth;

    uint8_t bitmask = 0x00;
    const int nw = (getValueByCoord(vec, x - 1, y - 1, mapWidth) & flag) == flag ? 1 : 0;
    const int n  = (getValueByCoord(vec, x, y - 1, mapWidth)     & flag) == flag ? 1 : 0;
    const int ne = (getValueByCoord(vec, x + 1, y - 1, mapWidth) & flag) == flag ? 1 : 0;
    const int w  = (getValueByCoord(vec, x - 1, y, mapWidth)     & flag) == flag ? 1 : 0;
    const int e  = (getValueByCoord(vec, x + 1, y, mapWidth)     & flag) == flag ? 1 : 0;
    const int sw = (getValueByCoord(vec, x - 1, y + 1, mapWidth) & flag) == flag ? 1 : 0;
    const int s  = (getValueByCoord(vec, x, y + 1, mapWidth)     & flag) == flag ? 1 : 0;
    const int se = (getValueByCoord(vec, x + 1, y + 1, mapWidth) & flag) == flag ? 1 : 0;

    if (n && w) bitmask +=   1 * nw;
    bitmask +=   2 * n;
    if (n && e) bitmask +=   4 * ne;
    bitmask +=   8 * w;
    bitmask +=  16 * e;
    if (s && w) bitmask +=  32 * sw;
    bitmask +=  64 * s;
    if (s && e) bitmask += 128 * se;

    return bitmask;
  }

  std::vector<float> GenerateHeightmap(uint32_t seed, int mapWidth, int mapHeight);
  std::vector<uint8_t> GenerateFeatureMap(uint32_t seed, std::vector<float> heightmap, int mapWidth, int mapHeight);
}

#endif //KINGDOM_WORLD_GENERATOR_H
