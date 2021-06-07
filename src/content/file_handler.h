#ifndef KINGDOM_FILE_HANDLER_H
#define KINGDOM_FILE_HANDLER_H

#include <string>
const char* ReadTextFile(const std::string& fileName);
const char* ReadBinaryFile(const std::string& fileName, size_t& fileSize);

#endif //KINGDOM_FILE_HANDLER_H
