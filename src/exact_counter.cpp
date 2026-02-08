#include "exact_counter.h"

size_t ExactCounter::count(const std::vector<std::string>& stream) {
  std::unordered_set<std::string> unique(stream.begin(), stream.end());
  return unique.size();
}