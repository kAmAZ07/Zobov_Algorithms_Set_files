#ifndef STREAM_GENERATOR_H
#define STREAM_GENERATOR_H

#include <string>
#include <random>
#include <vector>

class RandomStreamGen {
private:
    std::mt19937_64 rng;
    std::uniform_int_distribution<int> length_dist;
    std::uniform_int_distribution<int> char_dist;
    
    const std::string charset = 
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789-";

public:
    RandomStreamGen(uint64_t seed = std::random_device{}()) 
        : rng(seed), length_dist(1, 30), char_dist(0, charset.size() - 1) {}

    std::string generateString() {
        int len = length_dist(rng);
        std::string result;
        result.reserve(len);
        for (int i = 0; i < len; ++i) {
            result += charset[char_dist(rng)];
        }
        return result;
    }

    std::vector<std::string> generateStream(size_t size) {
        std::vector<std::string> stream;
        stream.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            stream.push_back(generateString());
        }
        return stream;
    }
};

#endif
