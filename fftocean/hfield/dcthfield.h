#pragma once

#include <complex>
#include <memory>

#include "base/basictypes.h"
#include "azer/math/math.h"
#include "azer/sandbox/water/fftocean/hfield/tile.h"

class DCTHField {
 public:
  DCTHField(Tile* tile, float unit_width);
  float length() const { return length_;}
  struct NodeInfo {
    std::complex<float> height;
  };

  template<class TVertex>
  void SimulateWave(float t, TVertex* data);
 private:
  void Calc(const azer::Vector2& x, float t, NodeInfo* info);
  float dispersion(int n, int m);
  float phillips(int n, int m);
  std::complex<float> tilde0(int n, int m);
  std::complex<float> tilde(float t , int n, int m);

  azer::Vector2 wind_;
  float length_;
  const float A; // phillips parameters
  const float G; // 9.8
  const int32 N;
  
  struct Node {
    std::complex<float> h0;
    std::complex<float> _h0;
  };
  std::shared_ptr<Node> nodes_;
  DISALLOW_COPY_AND_ASSIGN(DCTHField);
};

template<class TVertex>
void DCTHField::SimulateWave(float t, TVertex* data) {
  NodeInfo info;
  for (int m = 0; m < N; ++m) {
    for (int n = 0; n < N; ++n) {
      int index = m * (N + 1) + n;
      TVertex* v = data + index;
      azer::Vector2 x(v->position.x, v->position.z);
      Calc(x, t, &info);
      v->position.y = info.height.real();
    }
  }
}
