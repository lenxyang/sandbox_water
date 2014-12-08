#include "azer/sandbox/water/fftocean/fft/fft.h"

#include "azer/math/math.h"

const float FFT::kPI2 = azer::kPI * azer::kPI;

FFT::FFT(const int dim) 
    : which_(0)
    , kDim(dim)
    , kLogDim(std::log(dim) / std::log(2)) {
  reversed_.reset(new uint32[kDim]);
  for (int i = 0; i < kDim; ++i) {
    reversed_.get()[i] = reverse(i);
  }

  c[0] = new std::complex<float>[kDim];
  c[1] = new std::complex<float>[kDim];
  
  int pow2 = 1;
  for (int i = 0; i < kLogDim; ++i) {
    omega_.push_back(new std::complex<float>[pow2]);
    for (int j = 0; j < pow2; ++j) {
      omega_[i][j] = std::move(calc_omega(j, pow2 * 2));
    }
    pow2 *= 2;
  }
}

FFT::~FFT() {
  delete[] c[0];
  delete[] c[1];
  for (int i = 0; i < kLogDim; ++i) {
    delete omega_[i];
  }
}

std::complex<float> FFT::calc_omega(int x, int N) const {
  return std::complex<float>(std::cos(kPI2 * x/ N), sin(kPI2 * x / N));
}

const std::complex<float>& FFT::omega(int n, int m) const {
  return omega_[n][m];
}

uint32 FFT::reverse(uint32 index) {
  uint32 res = 0;
  for (int j = 0; j < kLogDim; ++j) {
    res = (res << 1) + (index & 1);
    index >>= 1;
  }
  return res;
}

void FFT::fft(std::complex<float>* input, std::complex<float>* output,
              int stride, int offset) {
  for (int i = 0; i < kDim; ++i) {
    c[which_][i] = input[reversed_.get()[i] * stride + offset];
  }

  int loops = kDim >> 1;
  int size = 1 << 1;
  int size_over_2 = 1;
  int w = 0;

  for (int i = 1; i < kLogDim; ++i) {
    which_ ^= 1;
    int cur = which_;
    int prev = which_ ^ 1;
    for (int j = 0; j < loops; ++j) {
      for (int k = 0; k < size_over_2; ++k) {
        int index = size * j + k;
        int index2 = size * j + size_over_2 + k;
        c[cur][index] = c[prev][index] + c[prev][index2] * omega(w, k);
      }
      for (int k = size_over_2; k < size; ++k) {
        int index = size * j + k;
        int index2 = size * j - size_over_2 + k;
		c[cur][index] = c[prev][index2] - c[prev][index] * omega(w, k - size_over_2);
      }
    }

    loops       >>= 1;
    size        <<= 1;
    size_over_2 <<= 1;
    w++;
  }
  for (int i = 0; i < kDim; ++i) {
    output[i * stride + offset] = c[which_][i];
  }
}
