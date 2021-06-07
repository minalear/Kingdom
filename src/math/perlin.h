#ifndef KINGDOM_PERLIN_H
#define KINGDOM_PERLIN_H

#include <vector>

class Perlin {
  std::vector<int> p;

  static float fade(float t);
  static float lerp(float t, float a, float b);
  static float grad(int hash, float x, float y, float z);

public:
  Perlin();
  explicit Perlin(uint32_t seed);

  float Noise(float x, float y, float z) const;
};

#endif //KINGDOM_PERLIN_H
