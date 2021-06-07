#ifndef KINGDOM_SPRITE_BATCH_H
#define KINGDOM_SPRITE_BATCH_H

#include <cstdint>
#include "texture_2d.h"
#include "shader_program.h"
#include "glm/glm.hpp"
#include "vertex_buffer.h"

class SpriteBatch {
  ShaderProgram shaderProgram;
  VertexBuffer vertexBuffer;

  void draw(const Texture2D& texture, const glm::mat4& model);

public:
  SpriteBatch(int bufferWidth, int bufferHeight);

  void Resize(int bufferWidth, int bufferHeight);

  void Begin(const Texture2D& texture) const;
  void End() const;

  void Draw(const Texture2D& texture, const glm::mat4& model);
  void Draw(const Texture2D& texture, const glm::vec2& pos);
  void Draw(const Texture2D& texture, const glm::vec2& pos, const glm::vec2& scale);
};

#endif //KINGDOM_SPRITE_BATCH_H
