#pragma once

#include <complex>
#include <memory>

#include "base/basictypes.h"
#include "azer/math/math.h"
#include "azer/sandbox/water/fftocean/lighted/tile.h"
#include "azer/sandbox/water/fftocean/lighted/fft.h"

class Ocean {
 public:
  Ocean(Tile* tile, float unit_width);
  float length() const { return length_;}
  struct NodeInfo {
    std::complex<float> height;
  };

  template<class TVertex>
  void SimulateWave(float t, TVertex* data);

  template<class TVertex>
  void SimulateWaveFFT(float t, TVertex* data);
 private:
  void Calc(const azer::Vector2& x, float t, NodeInfo* info);
  float dispersion(int n, int m);
  float phillips(int n, int m);
  std::complex<float> tilde0(int n, int m);
  std::complex<float> tilde(float t , int n, int m);

  std::unique_ptr<std::complex<float> > htilde_;
  std::unique_ptr<std::complex<float> > htilde_dx_;
  std::unique_ptr<std::complex<float> > htilde_dz_;
  std::unique_ptr<std::complex<float> > htilde_slopex_;
  std::unique_ptr<std::complex<float> > htilde_slopez_;

  azer::Vector2 wind_;
  float length_;
  FFT fft_;
  const float A; // phillips parameters
  const float G; // 9.8
  const int32 N;
  
  struct Node {
    std::complex<float> h0;
    std::complex<float> _h0;
  };
  std::shared_ptr<Node> nodes_;
  DISALLOW_COPY_AND_ASSIGN(Ocean);
};

template<class TVertex>
void Ocean::SimulateWave(float t, TVertex* data) {
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

template<class TVertex>
void Ocean::SimulateWaveFFT(float t, TVertex* data) {
  using azer::kPI;
  std::complex<float>* htilde = htilde_.get();
  std::complex<float>* htilde_slopex = htilde_slopex_.get();
  std::complex<float>* htilde_slopez = htilde_slopez_.get();
  std::complex<float>* htilde_dx = htilde_dx_.get();
  std::complex<float>* htilde_dz = htilde_dz_.get();
  NodeInfo info;
  for (int m = 0; m < N; ++m) {
    float kz = kPI * (2.0f * m - N) / length();
    for (int n = 0; n < N; ++n) {
      float kx = kPI * (2.0f * n - N) / length();
      float len = std::sqrt(kx * kx + kz * kz);
      int index = m * N + n;
      htilde[index] = tilde(t, n, m);
      htilde_slopex[index] = htilde[index] * std::complex<float>(0, kx);
      htilde_slopez[index] = htilde[index] * std::complex<float>(0, kz);
      if (len > 0.000001) {
        htilde_dx[index] = htilde[index] * std::complex<float>(0, -kx/len);
        htilde_dz[index] = htilde[index] * std::complex<float>(0, -kz/len);
      } else {
        htilde_dx[index] = std::complex<float>(0.0f, 0.0f);
        htilde_dz[index] = std::complex<float>(0.0f, 0.0f);
      }
    }
  }

  for (int m = 0; m < N; ++m) {
    fft_.fft(htilde, htilde, 1, m * N);
    fft_.fft(htilde_slopex, htilde_slopex, 1, m * N);
    fft_.fft(htilde_slopez, htilde_slopez, 1, m * N);
    fft_.fft(htilde_dx, htilde_dx, 1, m * N);
    fft_.fft(htilde_dz, htilde_dz, 1, m * N);
  }

  for (int n = 0; n < N; ++n) {
    fft_.fft(htilde, htilde, N, n);
    fft_.fft(htilde_slopex, htilde_slopex, N, n);
    fft_.fft(htilde_slopez, htilde_slopez, N, n);
    fft_.fft(htilde_dx, htilde_dx, N, n);
    fft_.fft(htilde_dz, htilde_dz, N, n);
  }

  float signs[] = {1.0f, -1.0f};
  for (int m = 0; m < N; ++m) {
    for (int n = 0; n < N; ++n) {
      int index = m * N + n;
      int index2 = m * (N + 1) + n;
      TVertex* v = data + index2;
      float height = htilde[index].real();
      float sign = signs[(n + m) & 1];
      v->position.y = height * sign;

      htilde_slopex[index] *= sign;
      htilde_slopez[index] *= sign;
      htilde_dx[index] *= sign;
      htilde_dz[index] *= sign;
      azer::Vector3 normal(0.0f - htilde_slopex[index].real(),
                           1.0f,
                           0.0f - htilde_slopez[index].real());
      v->normal = std::move(normal.NormalizeCopy());
    }
  }
}
