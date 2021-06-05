#ifndef KINGDOM_RNG_H
#define KINGDOM_RNG_H

namespace rng {
  void init();

  int next_int();
  int next_int(int max);
  int next_int(int min, int max);
}

#endif //KINGDOM_RNG_H
