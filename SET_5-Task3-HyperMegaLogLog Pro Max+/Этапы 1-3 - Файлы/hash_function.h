#ifndef HASH_FUNCTION_H
#define HASH_FUNCTION_H

#include <cstdint>
#include <string>
#include <random>

class HashFuncGen {
private:
    uint64_t seed1, seed2;

public:
    HashFuncGen(uint64_t s1 = 0x9e3779b97f4a7c15ULL, 
                uint64_t s2 = 0x517cc1b727220a95ULL) 
        : seed1(s1), seed2(s2) {}

    uint32_t hash(const std::string& key) const {
        uint64_t h = seed1;
        const uint64_t m = 0xc6a4a7935bd1e995ULL;
        const int r = 47;

        for (char c : key) {
            uint64_t k = static_cast<uint64_t>(static_cast<unsigned char>(c));
            k *= m;
            k ^= k >> r;
            k *= m;
            h ^= k;
            h *= m;
        }

        h ^= seed2;
        h ^= h >> r;
        h *= m;
        h ^= h >> r;

        return static_cast<uint32_t>(h ^ (h >> 32));
    }

    static HashFuncGen random(uint64_t seed = std::random_device{}()) {
        std::mt19937_64 rng(seed);
        return HashFuncGen(rng(), rng());
    }
};

#endif
