#include "sprite_batch.h"
#include "glad/glad.h"
#include "glm/gtc/matrix_transform.hpp"

const char* VERTEX_SHADER_SOURCE =
    "#version 400\n"
    "layout(location = 0) in vec2 aPos;\n"
    "layout(location = 1) in vec2 aUV;\n"
    "layout(location = 2) in vec3 aColor;\n"

    "out vec2 UV;\n"
    "out vec3 Color;\n"

    "uniform mat4 proj;\n"
    "uniform mat4 view;\n"
    "uniform mat4 model;\n"

    "void main() {\n"
    "  UV = aUV;\n"
    "  Color = aColor;\n"
    "  gl_Position = proj * view * model * vec4(aPos, 0.0, 1.0);\n"
    "}"
;
const char* FRAGMENT_SHADER_SOURCE =
    "#version 400\n"
    "in vec2 UV;\n"
    "in vec3 Color;\n"

    "out vec4 fragment_color;\n"

    "uniform sampler2D image;\n"

    "void main() {\n"
    "  vec4 sampled_color = texture(image, UV);\n"
    "  fragment_color = sampled_color * vec4(Color, 1.0);\n"
    "}"
;

void SpriteBatch::draw(const Texture2D &texture, const glm::mat4 &model) {

}

SpriteBatch::SpriteBatch(const int bufferWidth, const int bufferHeight)
    : shaderProgram(VERTEX_SHADER_SOURCE, FRAGMENT_SHADER_SOURCE)
{
  // Setup VAO and VBO objects for sprite rendering
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  float buffer[] = {
    // POS      UV          COLOR
    0.f, 0.f,   0.f, 0.f,   1.f, 1.f, 1.f,
    0.f, 1.f,   0.f, 1.f,   1.f, 1.f, 1.f,
    1.f, 0.f,   1.f, 0.f,   1.f, 1.f, 1.f,
    1.f, 0.f,   1.f, 0.f,   1.f, 1.f, 1.f,
    0.f, 1.f,   0.f, 1.f,   1.f, 1.f, 1.f,
    1.f, 1.f,   1.f, 1.f,   1.f, 1.f, 1.f
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0); // Position
  glEnableVertexAttribArray(1); // UV
  glEnableVertexAttribArray(2); // Color

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (GLvoid*)0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (GLvoid*)(2 * sizeof(float)));
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (GLvoid*)(4 * sizeof(float)));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

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
SpriteBatch::~SpriteBatch() {
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);
}

void SpriteBatch::Resize(const int bufferWidth, const int bufferHeight) { }

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