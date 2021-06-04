#ifndef KINGDOM_SHADER_PROGRAM_H
#define KINGDOM_SHADER_PROGRAM_H

#include <map>
#include "glm/glm.hpp"

class ShaderProgram {
  uint32_t program_id;

  std::map<const char*, int> uniform_map;
  int32_t GetUniformLocation(const char* name);

public:
  ShaderProgram(const char* vSource, const char* fSource);
  ~ShaderProgram();

  uint32_t Id() const;
  void Use() const;

  void SetUniform(const char* name, bool value);
  void SetUniform(const char* name, const glm::vec2 &value);
  void SetUniform(const char* name, const glm::vec3 &value);
  void SetUniform(const char* name, const glm::vec4 &value);
  void SetUniform(const char* name, const glm::mat4 &value);
};


#endif //KINGDOM_SHADER_PROGRAM_H
