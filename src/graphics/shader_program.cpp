#include <cstdio>
#include "shader_program.h"
#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"

bool checkShaderCompilation(uint32_t shader_id) {
  int compileStatus = GL_FALSE;
  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compileStatus);

  if (compileStatus == GL_FALSE) {
    GLchar errorMessage[512];
    glGetShaderInfoLog(shader_id, 512, nullptr, errorMessage);
    printf("Shader Compilation Error, %s\n", errorMessage);
  }

  return bool(compileStatus);
}
bool checkShaderLinkStatus(uint32_t program_id) {
  int linkStatus = GL_FALSE;
  glGetProgramiv(program_id, GL_LINK_STATUS, &linkStatus);

  if (linkStatus == GL_FALSE) {
    GLchar errorMessage[512];
    glGetProgramInfoLog(program_id, 512, nullptr, errorMessage);
    printf("Shader Program Link Error, %s\n", errorMessage);
  }

  return bool(linkStatus);
}

ShaderProgram::ShaderProgram(const char *vSource, const char *fSource) {
  program_id = glCreateProgram();

  // Vertex shader
  uint32_t vShaderId = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vShaderId, 1, &vSource, nullptr);
  glCompileShader(vShaderId);
  assert(checkShaderCompilation(vShaderId));

  // Fragment shader
  uint32_t fShaderId = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fShaderId, 1, &fSource, nullptr);
  glCompileShader(fShaderId);
  assert(checkShaderCompilation(fShaderId));

  // Attach shaders and link program
  glAttachShader(program_id, vShaderId);
  glAttachShader(program_id, fShaderId);
  glLinkProgram(program_id);

  // Check for linking errors
  assert(checkShaderLinkStatus(program_id));

  // Cleanup
  glDetachShader(program_id, vShaderId);
  glDetachShader(program_id, fShaderId);
  glDeleteShader(vShaderId);
  glDeleteShader(fShaderId);
}
ShaderProgram::~ShaderProgram() {
  glUseProgram(0);
  glDeleteProgram(program_id);
}

// Private methods
int32_t ShaderProgram::GetUniformLocation(const char *name) {
  // checks if the uniform exists, if not, inserts it into the map
  if (uniform_map.find(name) == uniform_map.end()) {
    int loc = glGetUniformLocation(program_id, name);
    uniform_map.insert({name, loc});

    return loc;
  }

  return uniform_map[name];
}

// Public methods
uint32_t ShaderProgram::Id() const {
  return program_id;
}
void ShaderProgram::Use() const {
  glUseProgram(program_id);
}

void ShaderProgram::SetUniform(const char *name, bool value) {
  glUniform1i(GetUniformLocation(name), value);
}
void ShaderProgram::SetUniform(const char *name, glm::vec2 &value) {
  glUniform2f(GetUniformLocation(name), value.x, value.y);
}
void ShaderProgram::SetUniform(const char *name, glm::vec3 &value) {
  glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
}
void ShaderProgram::SetUniform(const char *name, glm::vec4 &value) {
  glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
}
void ShaderProgram::SetUniform(const char *name, glm::mat4 &value) {
  glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}
