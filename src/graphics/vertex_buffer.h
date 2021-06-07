#ifndef KINGDOM_VERTEX_BUFFER_H
#define KINGDOM_VERTEX_BUFFER_H

#include <cstdint>
#include "glad/glad.h"

class VertexBuffer {
  uint32_t vao;
  uint32_t vbo;

public:
  VertexBuffer();
  ~VertexBuffer();

  void SetBufferData(const float *buffer, size_t size) const;
  void EnableVertexAttribute(GLint index) const;
  void VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid* pointer) const;
  void Bind() const;
  void Unbind() const;
};

#endif //KINGDOM_VERTEX_BUFFER_H
