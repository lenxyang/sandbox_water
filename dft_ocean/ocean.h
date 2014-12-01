#pragma once

#include "base/basictypes.h"
#include "azer/render/render.h"
#include "azer/sandbox/water/dft_ocean/tile.h"

#include <complex>

class Ocean {
 public:
  Ocean(int level)
      : tile_(level) {
  }

  typedef std::complex<float> complex;
  void Init();
 private:
  // calc phillips spectrum
  float phillips(int n, int m);
  complex CalcHTilde0(int n, int m);
  complex CalcHTilde(float t, int n, int m);

  // wind's volumn and direction, use vector2 for [x, z]
  azer::Vector2 wind_;
  Tile tile_;
  std::unique_ptr<complex[]> h_tilde_;
  static const float kG;
  const float kLength;
  const int kDim;
  DISALLOW_COPY_AND_ASSIGN(Ocean);
};
