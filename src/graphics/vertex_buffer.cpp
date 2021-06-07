#include "vertex_buffer.h"
#include "glad/glad.h"

VertexBuffer::VertexBuffer() {
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
}
VertexBuffer::~VertexBuffer() {
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);
}

void VertexBuffer::SetBufferData(const float *buffer, const size_t size) const {
  glBufferData(GL_ARRAY_BUFFER, size, buffer, GL_STATIC_DRAW);
}

void VertexBuffer::EnableVertexAttribute(GLint index) const {
  glEnableVertexAttribArray(index);
}

void VertexBuffer::VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid *pointer) const {
  glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

void VertexBuffer::Bind() const {
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
}

void VertexBuffer::Unbind() const {
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}
