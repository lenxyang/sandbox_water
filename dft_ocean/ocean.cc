#include "azer/sandbox/water/dft_ocean.h"

const float Ocean::kG = 9.8;

void Ocean::Init() {
  h_tile_.reset(new Complex[kDim * kDim]);
  for (int i = 0; i < tile_.GetGridLineNum(); ++i) {
    for (int j= 0; j < tile_.GetGridLineNum(); ++j) {
      int index = i * tile_.GetGridLineNum() + j;
      complex htilde0 = CalcHTilde0(i, j);
      complex htilde0mk_conj = CalcHTilde0(-i, -j).conj();
    }
  }
}

float Ocean::phillips(int kn, int km) {
}

complex Ocean::CalcHTilde0(int kn, int km) {
}

complex Ocean::CalcHTilde(float t, int n, int m) {
}
