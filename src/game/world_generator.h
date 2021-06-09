#ifndef KINGDOM_WORLD_GENERATOR_H
#define KINGDOM_WORLD_GENERATOR_H

#include <cstdint>
#include <vector>

std::vector<float> GenerateHeightmap(uint32_t seed, int mapWidth, int mapHeight);

#endif //KINGDOM_WORLD_GENERATOR_H
