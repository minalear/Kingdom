#include "sprite_batch.h"
#include "glm/gtc/matrix_transform.hpp"
#include "../content/file_handler.h"

SpriteBatch::SpriteBatch(const int bufferWidth, const int bufferHeight) {
  // Load shaders from files and initialize ShaderProgram
  const char* vShaderSource = ReadTextFile("content/shaders/texturedVS.glsl");
  const char* fShaderSource = ReadTextFile("content/shaders/texturedFS.glsl");
  shaderProgram.LoadShaderSources(vShaderSource, fShaderSource);

  // Setup VertexBuffer
  vertexBuffer.Bind();

  float buffer[] = {
    // POS      UV
    0.f, 0.f,   0.f, 0.f,
    0.f, 1.f,   0.f, 1.f,
    1.f, 0.f,   1.f, 0.f,
    1.f, 0.f,   1.f, 0.f,
    0.f, 1.f,   0.f, 1.f,
    1.f, 1.f,   1.f, 1.f,
  };
  vertexBuffer.SetBufferData(buffer, sizeof(buffer));

  vertexBuffer.EnableVertexAttribute(0); // Position
  vertexBuffer.EnableVertexAttribute(1); // UV

  vertexBuffer.VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)0);
  vertexBuffer.VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)(2 * sizeof(float)));

  vertexBuffer.Unbind();

  // Matrix uniforms setup
  glm::mat4 proj, view, model;
  proj = glm::ortho(0.f, float(bufferWidth), float(bufferHeight), 0.f, -1.f, 1.f);
  view = glm::mat4(1.f);
  model = glm::mat4(1.f);

  shaderProgram.Use();
  shaderProgram.SetUniform("proj", proj);
  shaderProgram.SetUniform("view", view);
  shaderProgram.SetUniform("model", model);
}

// Private methods
void SpriteBatch::draw(const Texture2D &texture, const glm::mat4 &model) {
  shaderProgram.SetUniform("model", model);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

// Public methods
void SpriteBatch::Resize(const int bufferWidth, const int bufferHeight) { }

void SpriteBatch::Begin(const Texture2D& texture) const {
  shaderProgram.Use();
  texture.Bind();
  vertexBuffer.Bind();
}
void SpriteBatch::End() const {
  vertexBuffer.Unbind();
}

void SpriteBatch::Draw(const Texture2D &texture, const glm::mat4 &model) {
  draw(texture, model);
}
void SpriteBatch::Draw(const Texture2D &texture, const glm::vec2 &pos) {
  glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(pos, 0.f));
  model = glm::scale(model, glm::vec3(texture.Width(), texture.Height(), 1.f));

  draw(texture, model);
}
void SpriteBatch::Draw(const Texture2D &texture, const glm::vec2 &pos, const glm::vec2 &scale) {
  glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(pos, 0.f));
  model = glm::scale(model, glm::vec3(texture.Width(), texture.Height(), 1.f) * glm::vec3(scale, 1.f));

  draw(texture, model);
}

ShaderProgram &SpriteBatch::GetShaderProgram() {
  return shaderProgram;
}
