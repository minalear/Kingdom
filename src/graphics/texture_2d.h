#ifndef KINGDOM_TEXTURE_2D_H
#define KINGDOM_TEXTURE_2D_H

#include <cstdint>

class Texture2D {
  uint32_t id;
  int width;
  int height;
  int nBits;

public:
  Texture2D(const char* path);
  ~Texture2D();

  int Width() const;
  int Height() const;
  void Bind() const;
};

#endif //KINGDOM_TEXTURE_2D_H
