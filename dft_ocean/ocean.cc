#include "azer/sandbox/water/dft_ocean.h"

const float Ocean::kG = 9.8;

void Ocean::Init() {
  h_tile_.reset(new Complex[kDim * kDim]);
  for (int i = 0; i <  N + 1; ++i) {
  }
}
