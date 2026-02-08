#include "random_stream_gen.h"
#include <algorithm>
#include <cassert>
#include <unordered_set>
#include <string>

static const std::string CHARSET =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789-";

RandomStreamGen::RandomStreamGen(size_t uniquePool, size_t totalSize, uint64_t seed)
    : uniquePool_(uniquePool), totalSize_(totalSize), seed_(seed) {
  assert(uniquePool > 0);
  assert(totalSize >= uniquePool);
}

std::string RandomStreamGen::generateRandomString(std::mt19937_64& rng) {
  std::uniform_int_distribution<size_t> lenDist(1, 30);
  std::uniform_int_distribution<size_t> charDist(0, CHARSET.size() - 1);

  size_t len = lenDist(rng);
  std::string s(len, ' ');
  for (size_t i = 0; i < len; ++i) {
    s[i] = CHARSET[charDist(rng)];
  }
  return s;
}

const std::vector<std::string>& RandomStreamGen::generate() {
  std::mt19937_64 rng(seed_);

  // ?????????? set ??? ???????? ????????????
  std::unordered_set<std::string> uniqueSet;
  while (uniqueSet.size() < uniquePool_) {
    uniqueSet.insert(generateRandomString(rng));
  }
  pool_.assign(uniqueSet.begin(), uniqueSet.end());

  stream_.resize(totalSize_);
  std::uniform_int_distribution<size_t> poolDist(0, pool_.size() - 1);
  for (size_t i = 0; i < totalSize_; ++i) {
    stream_[i] = pool_[poolDist(rng)];
  }

  return stream_;
}

std::vector<std::string> RandomStreamGen::getPrefix(double fraction) const {
  assert(fraction > 0.0 && fraction <= 1.0);
  size_t count = static_cast<size_t>(std::ceil(fraction * stream_.size()));
  count = std::min(count, stream_.size());
  return std::vector<std::string>(stream_.begin(), stream_.begin() + count);
}

std::vector<double> RandomStreamGen::makeFractions(double step) {
  std::vector<double> fracs;
  for (double f = step; f <= 1.0 + 1e-9; f += step) {
    fracs.push_back(std::min(f, 1.0));
  }
  return fracs;
}