#include "rng.h"
#include <cstdlib>
#include <ctime>
#include <cstdint>

void rng::init() {
  srand((uint32_t)time(nullptr));
}

int rng::next_int() {
  return rand();
}
int rng::next_int(int max) {
  return rand() % max;
}
int rng::next_int(int min, int max) {
  return rand() % (max - min) + min;
}