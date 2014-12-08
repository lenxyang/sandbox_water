#include "azer/sandbox/water/fftocean/fft/dcthfield.h"

#include "azer/math/math.h"
#include "base/rand_util.h"
#include <cmath>

#include <random>

using azer::kPI;

float uniformRandom() {
  return (float)rand()/RAND_MAX;
  // return ::base::RandDouble();
}

std::complex<float> gaussianRandom() {
  float x1, x2, w;
  do {
    x1 = 2.f * uniformRandom() - 1.f;
    x2 = 2.f * uniformRandom() - 1.f;
    w = x1 * x1 + x2 * x2;
  } while ( w >= 1.f );
  w = std::sqrt((-2.f * log(w)) / w);
  return std::complex<float>(x1 * w, x2 * w);
}

DCTHField::DCTHField(Tile* tile, float unit_width) 
    : wind_(azer::Vector2(0.0f, 32.0f))
    , length_(tile->cell_num() * unit_width)
    , A(0.0005f)
    , G(9.81f)
    , N(tile->cell_num())
	, fft_(tile->cell_num()) {
  nodes_.reset(new Node[(N + 1) * (N + 1)]);
  for (int n = 0; n < tile->grid_line_num(); ++n) {
    for (int m = 0; m < tile->grid_line_num(); ++m) {
      int index = m * tile->grid_line_num() + n;
      nodes_.get()[index].h0 = tilde0(n, m);
      nodes_.get()[index]._h0 = std::conj(tilde0(-n, -m));
    }
  }

  htilde_c_.reset(new std::complex<float>[N * N]);
}

float DCTHField::dispersion(int n, int m) {
  // w_0
  float w_0 = 2.0f * azer::kPI / 200.0f;
  float kx = kPI * (2 * n - N) / length();
  float kz = kPI * (2 * m - N) / length();
  return std::floor(std::sqrt(G * sqrt(kx * kx + kz * kz)) / w_0) * w_0;
}

float DCTHField::phillips(int n, int m) {
  azer::Vector2 k(kPI * (2 * n - N) / length(),
                  kPI * (2 * m - N) / length());
  float k_length = k.length();
  if (k_length < 0.00001) return 0.0f;
  float k_length2 = k_length * k_length;
  float k_length4 = k_length2 * k_length2;

  float k_dot_w = k.NormalizeCopy().dot(wind_.NormalizeCopy());
  float k_dot_w2 = k_dot_w * k_dot_w;

  float w_length = wind_.length();
  float L = w_length * w_length / G;
  float L2 = L * L;
  float damping = 0.001f;
  float l2 = L2 * damping * damping;
  return A * exp(-1.0f / (k_length2 * L2)) / k_length4 * k_dot_w2
      * exp(-k_length2 * l2);
}


std::complex<float> DCTHField::tilde0(int n, int m) {
  std::complex<float> r = std::move(gaussianRandom());
  return r * sqrt(phillips(n, m) / 2.0f);
}

std::complex<float> DCTHField::tilde(float t , int n, int m) {
  int index = m * (N + 1) + n;
  std::complex<float> htilde0(nodes_.get()[index].h0);
  std::complex<float> htilde0mk(nodes_.get()[index]._h0);
  float omegat = dispersion(n, m) * t;
  float cos_ = std::cos(omegat);
  float sin_ = std::sin(omegat);
  std::complex<float> c0(cos_, sin_);
  std::complex<float> c1(cos_, sin_);
  std::complex<float> res = htilde0 * c0 + htilde0mk * c1;
  return res;
}


void DCTHField::Calc(const azer::Vector2& x, float t, DCTHField::NodeInfo* info) {
  std::complex<float> h(0.0f, 0.0f);
  for (int m = 0; m < N; ++m) {
    float kz = 2.0f * kPI * (m - N / 2.0f) / length();
    for (int n = 0; n < N; ++n) {
      float kx = 2.0f * kPI * (n - N / 2.0f) / length();
      azer::Vector2 k(kx, kz);
      
      float k_length = k.length();
      float k_dot_x = k.dot(x);
      
      std::complex<float> c(std::cos(k_dot_x), std::sin(k_dot_x));
      std::complex<float> htilde_c = tilde(t, n, m) * c;
	  h += htilde_c;
    }
  }

  info->height = h;
}


