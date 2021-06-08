#include "func.h"

float lerp(float t, float x, float y) {
  return x + t * (y - x);
}
