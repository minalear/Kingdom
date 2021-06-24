#include "terrain.h"
#include <iostream>
#include <fstream>
#include <string>
#include "spdlog/spdlog.h"

using namespace std;

void splitLine(const string& buffer, vector<string> &strs) {
  size_t pos = buffer.find(' ');
  size_t initialPos = 0;

  while(pos != string::npos) {
    strs.push_back(buffer.substr(initialPos, pos - initialPos));
    initialPos = pos + 1;
    pos = buffer.find(' ', initialPos);
  }

  strs.push_back(buffer.substr(initialPos, std::min(pos, buffer.size()) - initialPos + 1));
}

uint8_t directionToBitmask(const string& dir) {
  if (dir == "NW")  return 0b00000001;
  if (dir == "N")   return 0b00000010;
  if (dir == "NE")  return 0b00000100;
  if (dir == "W")   return 0b00001000;
  if (dir == "E")   return 0b00010000;
  if (dir == "SW")  return 0b00100000;
  if (dir == "S")   return 0b01000000;
  if (dir == "SE")  return 0b10000000;
  if (dir == "0")   return 0b00000000;

  spdlog::error("Invalid direction type found in terrain file: {}", dir);
  return 0x00;
}

Terrain::Terrain(const std::string& path) {
  // Open .terrain file and parse terrain data from it
  fstream terrain_file;
  terrain_file.open(path, ios::in);

  string buffer;
  vector<string> tokens;
  size_t lineCount = 0; // for error reporting
  while (getline(terrain_file, buffer)) {
    lineCount++;

    // Read the file line by line and split the line by whitespace, then parse the data from the resulting list
    splitLine(buffer, tokens);
    if (tokens.size() < 3) {
      if (!buffer.empty())
        spdlog::error("Invalid data line ({}) in terrain file: {}", lineCount, path);
      tokens.clear();
      continue;
    }

    // data line =>   [terrain_type] [tile_index] [directions...]
    const string terrainType = tokens[0];
    const int tileIndex = stoi(tokens[1]);

    uint8_t bitmask = 0x00;
    for (int i = 2; i < tokens.size(); i++) {
      bitmask = bitmask | directionToBitmask(tokens[i]);
    }

    //spdlog::info("TERRAIN TYPE {} : INDEX {} : MASK {:b}", terrainType, tileIndex, bitmask);
    tokens.clear();

    // insert data into map
    if (terrainData.find(terrainType) == terrainData.end())
      terrainData.insert(std::pair(terrainType, std::map<uint8_t, int>()));
    terrainData[terrainType].insert(std::pair(bitmask, tileIndex));
  }

  terrain_file.close();
}

int Terrain::GetTileIndex(const string &terrain, uint8_t bitmask) {
  std::map<std::string, std::map<uint8_t, int>>::iterator terrain_it;
  terrain_it = terrainData.find(terrain);

  if (terrain_it != terrainData.end()) {
    std::map<uint8_t, int>::iterator tile_it;
    tile_it = terrain_it->second.find(bitmask);

    if (tile_it != terrain_it->second.end())
      return tile_it->second;
  }

  return -1;
}
