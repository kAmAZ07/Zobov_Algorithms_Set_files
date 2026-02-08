#ifndef HYPERLOGLOG_IMPROVED_H
#define HYPERLOGLOG_IMPROVED_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

class HyperLogLogImproved {
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

    double applyBiasCorrection(double raw_estimate, uint32_t zeros) const {
        double ratio = raw_estimate / m;
        double correction = 1.0;
        double zero_ratio = static_cast<double>(zeros) / m;
        
        if (ratio < 2.5) {
            if (zeros > 0) {
                return m * std::log(static_cast<double>(m) / zeros);
            }
            correction = 0.97;
        } else if (ratio < 5) {
            correction = 0.98;
        } else if (ratio < 10) {
            correction = 0.985;
        } else if (ratio < 20) {
            correction = 0.99;
        } else if (ratio < 40) {
            correction = 0.995;
        } else if (ratio < 60) {
            correction = 1.00;
        } else if (ratio < 80) {
            correction = 0.98;
        } else {
            correction = 0.96;
        }
        
        if (zero_ratio < 0.01) {
            correction *= 0.98;
        }
        
        return raw_estimate * correction;
    }

public:
    HyperLogLogImproved(uint32_t b_bits) : b(b_bits), m(1u << b_bits) {
        M.resize(m, 0);
        alpha_m = getAlphaM(m);
    }

    void add(uint32_t hash) {
        uint32_t j = hash >> (32 - b);
        uint32_t w = hash << b;
        M[j] = std::max(M[j], rho(w));
    }

    double estimate() const {
        double sum = 0.0;
        uint32_t zeros = 0;
        
        for (uint32_t i = 0; i < m; ++i) {
            sum += 1.0 / (1ull << M[i]);
            if (M[i] == 0) zeros++;
        }
        
        double raw_estimate = alpha_m * m * m / sum;
        double corrected = applyBiasCorrection(raw_estimate, zeros);
        if (corrected > (1.0 / 30.0) * (1ull << 32)) {
            return -(1ull << 32) * std::log(1.0 - corrected / (1ull << 32));
        }
        
        return corrected;
    }

    void reset() {
        std::fill(M.begin(), M.end(), 0);
    }

    double estimateError() const {
        return 1.04 / std::sqrt(m);
    }

    uint32_t getUsedRegisters() const {
        uint32_t used = 0;
        for (uint32_t i = 0; i < m; ++i) {
            if (M[i] > 0) used++;
        }
        return used;
    }

    size_t getMemoryUsage() const {
        return M.size() * sizeof(uint8_t);
    }
};

class HyperLogLogCompact {
private:
    uint32_t b;
    uint32_t m;
    std::vector<uint32_t> M_packed;
    static constexpr uint8_t BITS_PER_REGISTER = 6;
    static constexpr uint8_t MAX_REGISTER_VALUE = (1 << BITS_PER_REGISTER) - 1;
    double alpha_m;

    double getAlphaM(uint32_t m) const {
        if (m == 16) return 0.673;
        if (m == 32) return 0.697;
        if (m == 64) return 0.709;
        return 0.7213 / (1.0 + 1.079 / m);
    }

    uint8_t rho(uint32_t w) const {
        if (w == 0) return std::min<uint8_t>(32 - b + 1, MAX_REGISTER_VALUE);
        
        uint8_t leading_zeros = 0;
        for (int i = 31 - b; i >= 0; --i) {
            if (w & (1u << i)) {
                break;
            }
            leading_zeros++;
        }
        uint8_t result = leading_zeros + 1;
        return std::min(result, MAX_REGISTER_VALUE);
    }

    void setRegister(uint32_t index, uint8_t value) {
        uint32_t bits_per_uint32 = 32 / BITS_PER_REGISTER;
        uint32_t uint32_index = index / bits_per_uint32;
        uint32_t bit_offset = (index % bits_per_uint32) * BITS_PER_REGISTER;
        
        uint32_t mask = ((1u << BITS_PER_REGISTER) - 1) << bit_offset;
        M_packed[uint32_index] = (M_packed[uint32_index] & ~mask) | 
                                 (static_cast<uint32_t>(value) << bit_offset);
    }

    uint8_t getRegister(uint32_t index) const {
        uint32_t bits_per_uint32 = 32 / BITS_PER_REGISTER;
        uint32_t uint32_index = index / bits_per_uint32;
        uint32_t bit_offset = (index % bits_per_uint32) * BITS_PER_REGISTER;
        
        return (M_packed[uint32_index] >> bit_offset) & ((1u << BITS_PER_REGISTER) - 1);
    }

public:
    HyperLogLogCompact(uint32_t b_bits) : b(b_bits), m(1u << b_bits) {
        uint32_t bits_per_uint32 = 32 / BITS_PER_REGISTER;
        uint32_t num_uint32s = (m + bits_per_uint32 - 1) / bits_per_uint32;
        M_packed.resize(num_uint32s, 0);
        alpha_m = getAlphaM(m);
    }

    void add(uint32_t hash) {
        uint32_t j = hash >> (32 - b);
        uint32_t w = hash << b;
        uint8_t new_val = rho(w);
        uint8_t old_val = getRegister(j);
        if (new_val > old_val) {
            setRegister(j, new_val);
        }
    }

    double estimate() const {
        double sum = 0.0;
        uint32_t zeros = 0;
        uint32_t saturated = 0;
        
        for (uint32_t i = 0; i < m; ++i) {
            uint8_t reg_val = getRegister(i);
            sum += 1.0 / (1ull << reg_val);
            if (reg_val == 0) zeros++;
            if (reg_val == MAX_REGISTER_VALUE) saturated++;
        }
        
        double raw_estimate = alpha_m * m * m / sum;
        
        if (raw_estimate <= 2.5 * m && zeros != 0) {
            return m * std::log(static_cast<double>(m) / zeros);
        }
        
        double saturation_factor = static_cast<double>(saturated) / m;
        if (saturation_factor > 0.01) {
            raw_estimate *= (1.0 + saturation_factor * 0.5);
        }
        
        raw_estimate *= 0.99;
        if (raw_estimate > (1.0 / 30.0) * (1ull << 32)) {
            return -(1ull << 32) * std::log(1.0 - raw_estimate / (1ull << 32));
        }
        
        return raw_estimate;
    }

    void reset() {
        std::fill(M_packed.begin(), M_packed.end(), 0);
    }

    size_t getMemoryUsage() const {
        return M_packed.size() * sizeof(uint32_t);
    }
};

#endif
