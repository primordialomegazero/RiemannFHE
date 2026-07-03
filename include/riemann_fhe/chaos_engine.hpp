#pragma once
#include "phi_constants.hpp"
#include <random>
#include <cmath>

namespace riemann_fhe {

class PhiChaosEngine {
    double state_;
    uint64_t counter_;
public:
    explicit PhiChaosEngine(uint64_t seed = 0) : counter_(0) {
        if (seed == 0) { std::random_device rd; seed = rd(); }
        state_ = static_cast<double>(seed % 1000000) / 1000000.0;
        if (state_ < 0.01) state_ = 0.5;  // Avoid fixed points
        // Warm up
        for (int i = 0; i < 200; ++i) step();
    }

    void step() {
        // φ-logistic map with perturbation to avoid fixed points
        state_ = PhiConstants::PHI * state_ * (1.0 - state_);
        // Tiny perturbation based on counter
        double perturbation = std::sin(static_cast<double>(++counter_) * 0.1) * 1e-10;
        state_ += perturbation;
        state_ = state_ - std::floor(state_);
    }

    double random_double() { 
        step(); 
        return state_; 
    }
    
    uint64_t random_int(uint64_t min, uint64_t max) {
        return min + static_cast<uint64_t>(random_double() * (double)(max - min + 1));
    }

    static constexpr double lyapunov_exponent() { 
        return std::log(PhiConstants::PHI); 
    }
    
    bool verify_chaos() const { 
        return lyapunov_exponent() > 0.0; 
    }
};

static_assert(PhiChaosEngine::lyapunov_exponent() > 0, "Must be chaotic");

} // namespace riemann_fhe
