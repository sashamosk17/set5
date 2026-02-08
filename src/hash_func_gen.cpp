#include "hash_func_gen.h"

HashFuncGen::HashFuncGen(uint32_t seed) : seed_(seed) {
}

static inline uint32_t murmur3_scramble(uint32_t k) {
  k *= 0xcc9e2d51;
  k = (k << 15) | (k >> 17);
  k *= 0x1b873593;
  return k;
}

uint32_t HashFuncGen::hash(const std::string& key) const {
  uint32_t h = seed_;
  const uint8_t* data = reinterpret_cast<const uint8_t*>(key.data());
  size_t len = key.size();

  size_t nblocks = len / 4;
  for (size_t i = 0; i < nblocks; ++i) {
    uint32_t k;
    std::memcpy(&k, data + i * 4, sizeof(uint32_t));
    h ^= murmur3_scramble(k);
    h = (h << 13) | (h >> 19);
    h = h * 5 + 0xe6546b64;
  }

  uint32_t k = 0;
  const uint8_t* tail = data + nblocks * 4;
  switch (len & 3) {
    case 3:
      k ^= tail[2] << 16;
      [[fallthrough]];
    case 2:
      k ^= tail[1] << 8;
      [[fallthrough]];
    case 1:
      k ^= tail[0];
      h ^= murmur3_scramble(k);
  }

  h ^= static_cast<uint32_t>(len);
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;

  return h;
}

std::function<uint32_t(const std::string&)> HashFuncGen::getHashFunc() const {
  uint32_t s = seed_;
  return [gen = *this](const std::string& key) { return gen.hash(key); };
}