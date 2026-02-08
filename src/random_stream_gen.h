#pragma once

#include <string>
#include <vector>
#include <random>
#include <cstdint>

class RandomStreamGen {
 public:
  RandomStreamGen(size_t uniquePool, size_t totalSize, uint64_t seed = 42);

  const std::vector<std::string>& generate();
  std::vector<std::string> getPrefix(double fraction) const;

  static std::vector<double> makeFractions(double step = 0.05);

  const std::vector<std::string>& getStream() const {
    return stream_;
  }
  size_t totalSize() const {
    return totalSize_;
  }

 private:
  std::string generateRandomString(std::mt19937_64& rng);

  size_t uniquePool_;
  size_t totalSize_;
  uint64_t seed_;
  std::vector<std::string> pool_; // уникальные строки
  std::vector<std::string> stream_;  // итоговый поток
};