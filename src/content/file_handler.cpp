#include "file_handler.h"
#include <fstream>

/// Reads the contents of a file into a C-String
const char *ReadTextFile(const std::string& fileName) {
  std::fstream file(fileName);
  std::string line, output;
  while (getline(file, line)) {
    output.append(line);
    output.push_back('\n');
  }

  char *cStr = new char[output.size() + 1];
  for (int i = 0; i < output.size(); i++) {
    cStr[i] = output[i];
  }
  cStr[output.size()] = '\0';
  file.close();

  return cStr;
}

const char *ReadBinaryFile(const std::string& fileName, size_t& fileSize) {
  std::ifstream file(fileName, std::ios::binary | std::ios::ate);
  file.seekg(0, std::ios::end);
  fileSize = size_t(file.tellg());
  file.seekg(0, std::ios::beg);

  char* data = new char[fileSize];
  file.read(data, fileSize);

  file.close();
  return data;
}
