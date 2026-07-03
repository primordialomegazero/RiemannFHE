#pragma once
// ============================================================
// Rate Limiter (Adapted for RiemannFHE)
// Token bucket algorithm with φ-weighted refill
// φΩ0 — Primordial Omega Zero
// ============================================================
#include <map>
#include <string>
#include <mutex>
#include <chrono>
#include <atomic>

namespace riemann_rate {

constexpr double PHI = 1.61803398874989484820;
constexpr double PHI_INV = 0.61803398874989484820;

struct RateLimit {
    double tokens;
    double max_tokens;
    double refill_rate;  // tokens per second
    std::chrono::steady_clock::time_point last_refill;
};

class RateLimiter {
private:
    std::mutex mutex_;
    std::map<std::string, RateLimit> buckets_;
    double default_max_;
    double default_refill_;
    
public:
    RateLimiter(double max_tokens = 100.0, double refill_per_sec = 10.0)
        : default_max_(max_tokens), default_refill_(refill_per_sec) {}
    
    bool allow_request(const std::string& client_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto& bucket = buckets_[client_id];
        auto now = std::chrono::steady_clock::now();
        
        // Initialize new buckets
        if (bucket.max_tokens == 0) {
            bucket.tokens = default_max_;
            bucket.max_tokens = default_max_;
            bucket.refill_rate = default_refill_;
            bucket.last_refill = now;
        }
        
        // Refill tokens (φ-weighted for priority clients)
        double elapsed = std::chrono::duration<double>(now - bucket.last_refill).count();
        bucket.tokens += elapsed * bucket.refill_rate;
        if (bucket.tokens > bucket.max_tokens) bucket.tokens = bucket.max_tokens;
        bucket.last_refill = now;
        
        // Consume token
        if (bucket.tokens >= 1.0) {
            bucket.tokens -= 1.0;
            return true;
        }
        
        return false;  // Rate limited
    }
    
    // φ-scaled rate for priority users
    void set_priority_limit(const std::string& client_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& bucket = buckets_[client_id];
        bucket.max_tokens = default_max_ * PHI;
        bucket.refill_rate = default_refill_ * PHI;
        bucket.tokens = bucket.max_tokens;
    }
    
    // Inverse φ for restricted users
    void set_restricted_limit(const std::string& client_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& bucket = buckets_[client_id];
        bucket.max_tokens = default_max_ * PHI_INV;
        bucket.refill_rate = default_refill_ * PHI_INV;
        bucket.tokens = bucket.max_tokens;
    }
    
    size_t active_clients() const {
        return buckets_.size();
    }
};

} // namespace riemann_rate
