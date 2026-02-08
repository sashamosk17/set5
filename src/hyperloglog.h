#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <functional>
#include <cmath>

class HyperLogLog {
 public:
  explicit HyperLogLog(int B, std::function<uint32_t(const std::string&)> hashFunc);
  void add(const std::string& item);
  double estimate() const;
  double estimateBeta() const;  // LogLog-Beta
  void reset();

  int getB() const {
    return B_;
  }
  int getM() const {
    return m_;
  }

 private:
  int B_;  
  int m_;  // ????? ????????? = 2^B
  double alpha_;  // ?????????????? ?????????
  std::vector<uint8_t> registers_;
  std::function<uint32_t(const std::string&)> hashFunc_;

  // ??????? ????? ??????? ????? + 1 ? ?????????? (32 - B) ?????
  uint8_t rho(uint32_t w) const;
  static double computeAlpha(int m);
  const std::vector<uint8_t>& getRegisters() const {
    return registers_;
  }
};