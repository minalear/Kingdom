#ifndef KINGDOM_SPRITE_BATCH_H
#define KINGDOM_SPRITE_BATCH_H

#include <cstdint>
#include "texture_2d.h"
#include "shader_program.h"
#include "glm/glm.hpp"

class SpriteBatch {
  uint32_t vao;
  uint32_t vbo;
  ShaderProgram shaderProgram;

  void draw(const Texture2D& texture, const glm::mat4& model);

public:
  SpriteBatch(const int bufferWidth, const int bufferHeight);
  ~SpriteBatch();

  void Resize(const int bufferWidth, const int bufferHeight);

  void Draw(const Texture2D& texture, const glm::mat4& model);
  void Draw(const Texture2D& texture, const glm::vec2& pos);
  void Draw(const Texture2D& texture, const glm::vec2& pos, const glm::vec2& scale);
};

#endif //KINGDOM_SPRITE_BATCH_H
