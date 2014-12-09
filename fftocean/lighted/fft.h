#pragma once

#include <cmath>
#include <memory>
#include <complex>
#include <vector>

#include "base/basictypes.h"

class FFT {
 public:
  FFT(const int dim);
  ~FFT();
  
  void fft(std::complex<float>* input, std::complex<float>* output,
           int stride, int offset);
 private:
  std::complex<float> calc_omega(int x, int N) const;
  const std::complex<float>& omega(int n, int m) const;

  // 用于计算排序好的下标(按照 FFT 的合并顺序排序)
  uint32 reverse(uint32 index);
  std::unique_ptr<uint32> reversed_;

  // c 用于保存中间计算结果，定义为 2 为数组是交替进行计算
  // which 用于区别当前用于计算和存储结果的下标
  int32 which_;
  std::complex<float>* c[2];
  
  std::vector<std::complex<float>* > omega_;
  const int kDim;
  const int kLogDim;
  static const float kPI2;
  DISALLOW_COPY_AND_ASSIGN(FFT);
};
