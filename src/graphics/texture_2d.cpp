#include "texture_2d.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"
#include "glad/glad.h"

Texture2D::Texture2D(const char *path) {
  auto data = stbi_load(path, &width, &height, &nBits, STBI_rgb_alpha);

  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
               0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  // glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0); // unbind texture
  stbi_image_free(data); // free texture data
}
Texture2D::~Texture2D() {
  glDeleteTextures(1, &id);
}

int Texture2D::Width() const {
  return width;
}
int Texture2D::Height() const {
  return height;
}
void Texture2D::Bind() const {
  glBindTexture(GL_TEXTURE_2D, id);
}
