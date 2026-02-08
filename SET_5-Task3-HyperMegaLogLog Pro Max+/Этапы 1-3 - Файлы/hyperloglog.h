#ifndef HYPERLOGLOG_H
#define HYPERLOGLOG_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

class HyperLogLog {
private:
    uint32_t b;
    uint32_t m;
    std::vector<uint8_t> M;
    double alpha_m;

    double getAlphaM(uint32_t m) const {
        if (m == 16) return 0.673;
        if (m == 32) return 0.697;
        if (m == 64) return 0.709;
        return 0.7213 / (1.0 + 1.079 / m);
    }

    uint8_t rho(uint32_t w) const {
        if (w == 0) return 32 - b + 1;
        
        uint8_t leading_zeros = 0;
        for (int i = 31 - b; i >= 0; --i) {
            if (w & (1u << i)) {
                break;
            }
            leading_zeros++;
        }
        return leading_zeros + 1;
    }

public:
    HyperLogLog(uint32_t b_bits) : b(b_bits), m(1u << b_bits) {
        M.resize(m, 0);
        alpha_m = getAlphaM(m);
    }

    void add(uint32_t hash) {
        uint32_t j = hash >> (32 - b);
        
        uint32_t w = hash << b;
        
        M[j] = std::max(M[j], rho(w));
    }

    double estimate() const {
        double raw_estimate = alpha_m * m * m / getSum();
        
        if (raw_estimate <= 2.5 * m) {
            uint32_t zeros = 0;
            for (uint32_t i = 0; i < m; ++i) {
                if (M[i] == 0) zeros++;
            }
            if (zeros != 0) {
                return m * std::log(static_cast<double>(m) / zeros);
            }
        }
        
        if (raw_estimate <= (1.0 / 30.0) * (1ull << 32)) {
            return raw_estimate;
        } else {
            return -(1ull << 32) * std::log(1.0 - raw_estimate / (1ull << 32));
        }
    }

    void reset() {
        std::fill(M.begin(), M.end(), 0);
    }

private:
    double getSum() const {
        double sum = 0.0;
        for (uint32_t i = 0; i < m; ++i) {
            sum += 1.0 / (1ull << M[i]);
        }
        return sum;
    }
};

#endif