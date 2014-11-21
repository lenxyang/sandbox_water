#pragma once

#include "base/basictypes.h"
#include "azer/render/render.h"
#include "azer/util/tile.h"

#include <complex>

class Ocean {
 public:
  typedef std::complex<float> complex;
  void Init();
 private:
  // calc phillips spectrum
  float phillips(int n, int m);
  complex CalcHTilde0(int n, int m);
  complex CalcHTilde(float t, int n, int m);

  std::unique_ptr<complex[]> h_tilde_;
  static const float kG;
  const float kLength;
  const int kDim;
  DISALLOW_COPY_AND_ASSIGN(Ocean);
};
