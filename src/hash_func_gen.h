#pragma once

#include <string>
#include <cstdint>
#include <functional>

class HashFuncGen {
 public:
  explicit HashFuncGen(uint32_t seed = 0);

  uint32_t hash(const std::string& key) const;
  std::function<uint32_t(const std::string&)> getHashFunc() const;

 private:
  uint32_t seed_;
};