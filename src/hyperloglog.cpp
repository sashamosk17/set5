#include "hyperloglog.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <cstring>

HyperLogLog::HyperLogLog(int B, std::function<uint32_t(const std::string&)> hashFunc)
    : B_(B), m_(1 << B), alpha_(computeAlpha(m_)), registers_(m_, 0), hashFunc_(std::move(hashFunc)) {
}

double HyperLogLog::computeAlpha(int m) {
  switch (m) {
    case 16:
      return 0.673;
    case 32:
      return 0.697;
    case 64:
      return 0.709;
    default:
      return 0.7213 / (1.0 + 1.079 / m);
  }
}

uint8_t HyperLogLog::rho(uint32_t w) const {
  if (w == 0) {
    return static_cast<uint8_t>(32 - B_ + 1);
  }
  uint32_t mask = 1u << (31 - B_);

  uint8_t leadingZeros = 0;
  while ((w & mask) == 0 && leadingZeros < (32 - B_)) {
    leadingZeros++;
    mask >>= 1;  // ????????? ????????? ???
  }
  return leadingZeros + 1;
}

void HyperLogLog::add(const std::string& item) {
  uint32_t h = hashFunc_(item);
  uint32_t idx = h >> (32 - B_);

  // ?????: (1 << (32 - B)) - 1 ???? (32-B) ?????? ? ??????? ?????
  uint32_t w = h & ((1u << (32 - B_)) - 1);

  uint8_t r = rho(w);

  if (r > registers_[idx]) {
    registers_[idx] = r;
  }
}
double HyperLogLog::estimateBeta() const {
  int m = m_;

  double sum = 0.0;
  int zeros = 0;

  for (int j = 0; j < m; ++j) {
    if (registers_[j] == 0) {
      zeros++;
    }
    sum += std::pow(2.0, -static_cast<double>(registers_[j]));
  }

  if (zeros == 0) {
    return estimate();
  }

  double ez = static_cast<double>(zeros);
  double zl = std::log(ez + 1);

  // ?????????????? ????????????? beta(z) ?? LogLog-Beta
  double beta = -0.370393911 * ez + 0.070471823 * zl + 0.17393686 * zl * zl + 0.16339839 * zl * zl * zl -
                0.09237745 * zl * zl * zl * zl + 0.03738027 * zl * zl * zl * zl * zl -
                0.005384159 * zl * zl * zl * zl * zl * zl + 0.00042419 * zl * zl * zl * zl * zl * zl * zl;

  double alpha_inf = 0.7213 / (1.0 + 1.079 / m);
  return alpha_inf * m * (m - zeros) / (sum + beta);
}
double HyperLogLog::estimate() const {
  double sum = 0.0;
  for (int j = 0; j < m_; ++j) {
    sum += std::pow(2.0, -static_cast<double>(registers_[j]));
  }

  double raw = alpha_ * m_ * m_ / sum;

  // ????????? ??? ????? ???????? (Linear Counting)
  if (raw <= 2.5 * m_) {
    int zeros = 0;
    for (int j = 0; j < m_; ++j) {
      if (registers_[j] == 0)
        zeros++;
    }
    if (zeros > 0) {
      raw = m_ * std::log(static_cast<double>(m_) / zeros);
    }
  }

  const double two32 = 4294967296.0;  // 2^32
  if (raw > two32 / 30.0) {
    raw = -two32 * std::log(1.0 - raw / two32);
  }

  return raw;
}

void HyperLogLog::reset() {
  std::fill(registers_.begin(), registers_.end(), 0);
}