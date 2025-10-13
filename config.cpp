#include "config.h"

int addOne(int x) {return x + 1;}
int zeroSwapTen(int x) {return x ? x : 10;}

FlipperConfig configs[NUM_FLIPPERS] = {
  {4,  15,  2, 33, 60, addOne},      // HOURS
  {23, 22, 21, 32, 20, zeroSwapTen}, // TENS
  {18,  5, 19, 35, 20, zeroSwapTen}  // ONES
};
