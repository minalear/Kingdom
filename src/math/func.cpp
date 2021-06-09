#include "func.h"
#include <cmath>

float lerp(float t, float x, float y) {
  return x + t * (y - x);
}

float dist(float x1, float y1, float x2, float y2) {
  return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}
