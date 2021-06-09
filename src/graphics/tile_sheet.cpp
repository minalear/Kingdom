#include "tile_sheet.h"
#include "stb_image.h"
#include "glad/glad.h"
#include "spdlog/spdlog.h"


TileSheet::TileSheet(const char *path, int tileSize)
    : tileSize(tileSize)
{
  auto data = stbi_load(path, &textureWidth, &textureHeight, nullptr, STBI_rgb_alpha);

  width = textureWidth / tileSize;
  height = textureHeight / tileSize;

  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight,
               0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  // glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0);
  stbi_image_free(data); // free texture data

  spdlog::info("Tilesheet created: {} id: {}, ({}x{})",
               path, id, textureWidth, textureHeight);
}

TileSheet::~TileSheet() {
  glDeleteTextures(1, &id);
}

int TileSheet::TileSize() const {
  return tileSize;
}

int TileSheet::TextureWidth() const {
  return textureWidth;
}

int TileSheet::TextureHeight() const {
  return textureHeight;
}

int TileSheet::Width() const {
  return width;
}

int TileSheet::Height() const {
  return height;
}

void TileSheet::Bind() const {
  glBindTexture(GL_TEXTURE_2D, id);
}
