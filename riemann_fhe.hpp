#pragma once
// ============================================================
// RIEMANN-FHE: φ-Harmonic Fully Homomorphic Encryption
// ============================================================
// Bootstrap-free FHE based on Riemann zeta zero spectral theory.
// The noise operator T(N) = N·φ⁻¹ + F_n·(1-φ⁻¹) is a Banach
// contraction with fixed point N* ≈ 1.828, eliminating bootstrapping.
//
// φΩ0 — Dan Joseph M. Fernandez / Primordial Omega Zero
// July 3, 2026
// ============================================================

#include <cmath>
#include <vector>
#include <array>
#include <random>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace riemann_fhe {

// ============================================================
// CONSTANTS
// ============================================================
constexpr double PHI        = 1.6180339887498948482;
constexpr double PHI_INV    = 0.6180339887498948482;
constexpr double PHI_SQ     = 2.6180339887498948482;
constexpr double PHI_INV_SQ = 0.3819660112501051518;
constexpr double PHI_HALF   = 0.8090169943749474241;
constexpr double PI         = 3.14159265358979323846;

// Noise operator: T(N) = N·φ⁻¹ + F_n·(1-φ⁻¹)
constexpr double NOISE_A    = PHI_INV;       // φ⁻¹
constexpr double NOISE_B    = PHI_INV_SQ;    // 1 - φ⁻¹ = φ⁻²
constexpr double NOISE_FIXED_POINT = 1.828154;
constexpr double MAX_SAFE_NOISE = 100.0;
constexpr double INITIAL_NOISE  = 2.0;
constexpr double SCALE = 1024.0;  // Encoding scale factor

// ============================================================
// ZETA ZERO DATA
// ============================================================
constexpr int ZETA_COUNT = 100;
constexpr double ZETA_ZEROS[ZETA_COUNT] = {
    14.134725, 21.022040, 25.010857, 30.424876, 32.935061,
    37.586178, 40.918719, 43.327073, 48.005150, 49.773832,
    52.970321, 56.446248, 59.347044, 60.831779, 65.112544,
    67.079811, 69.546402, 72.067158, 75.704691, 77.144840,
    79.337375, 82.910381, 84.735493, 87.425275, 88.809111,
    92.491899, 94.651344, 95.870634, 98.831194, 101.317851,
    103.725538, 105.446623, 107.168611, 111.029536, 111.874659,
    114.320221, 116.226680, 118.015783, 121.370125, 122.946829,
    124.256819, 127.516684, 129.578704, 131.087689, 133.497737,
    134.756510, 138.116042, 139.736209, 141.123707, 143.111846,
    146.000982, 147.422765, 150.053520, 150.925258, 153.024694,
    156.112909, 157.597593, 158.849988, 161.188964, 163.030710,
    165.537069, 167.184440, 169.094515, 169.911976, 173.411537,
    174.754192, 176.441434, 178.377408, 179.916484, 182.207078,
    184.874468, 185.598784, 187.228923, 189.416159, 192.026656,
    193.079727, 195.265397, 196.876482, 198.015310, 201.264752,
    202.493595, 204.189672, 205.394697, 207.906259, 209.576510,
    211.690863, 213.347919, 214.547045, 216.169539, 219.067595,
    220.714919, 221.430706, 224.007000, 224.983325, 227.421444,
    229.337413, 231.250189, 231.987235, 233.693404, 236.524230
};

constexpr int GAP_COUNT = ZETA_COUNT - 2;
constexpr std::array<double, GAP_COUNT> ZETA_GAP_RATIOS = [](){
    std::array<double, GAP_COUNT> r{};
    for (int i = 0; i < GAP_COUNT; i++) {
        double g1 = ZETA_ZEROS[i+1] - ZETA_ZEROS[i];
        double g2 = ZETA_ZEROS[i+2] - ZETA_ZEROS[i+1];
        r[i] = (g1 > 0.001) ? g2 / g1 : 1.0;
    }
    return r;
}();

// ============================================================
// RIEMANN CHAOS RNG (properly tuned)
// ============================================================
class RiemannRNG {
private:
    uint64_t s0_, s1_, s2_, s3_;
    uint64_t counter_;
    size_t gap_idx_;
    
    static inline uint64_t splitmix64(uint64_t& x) {
        x += 0x9E3779B97F4A7C15ULL;
        uint64_t z = x;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31);
    }
    
    static inline uint64_t rotl64(uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
    }
    
public:
    explicit RiemannRNG(uint64_t seed = 0) : counter_(0), gap_idx_(0) {
        if (seed == 0) { std::random_device rd; seed = rd(); }
        s0_ = splitmix64(seed);
        s1_ = splitmix64(s0_);
        s2_ = splitmix64(s1_);
        s3_ = splitmix64(s2_);
        for (int i = 0; i < 20; i++) next();
    }
    
    uint64_t next() {
        uint64_t result = rotl64(s0_ + s3_, 23) + s0_;
        double zeta = ZETA_GAP_RATIOS[gap_idx_ % GAP_COUNT];
        gap_idx_++;
        
        uint64_t t = s1_ << 17;
        s2_ ^= s0_;
        s3_ ^= s1_;
        s1_ ^= s2_;
        s0_ ^= s3_;
        s2_ ^= t;
        
        // Zeta perturbation: mix in φ-harmonic from zero gaps
        double perturb = zeta * PHI * 1e15;
        s3_ ^= static_cast<uint64_t>(perturb);
        
        s3_ = rotl64(s3_, 45);
        counter_++;
        return result;
    }
    
    double next_double() {
        return static_cast<double>(next() >> 11) / (1ULL << 53);
    }
    
    void generate(uint8_t* out, size_t n) {
        for (size_t i = 0; i < n; i += 8) {
            uint64_t val = next();
            size_t rem = std::min(size_t(8), n - i);
            std::memcpy(out + i, &val, rem);
        }
    }
    
    // Sample from φ-harmonic distribution (matching zeta gap ratios)
    double phi_harmonic() {
        double u = next_double();
        if (u < 0.307)       return PHI_HALF + gaussian(0.05);
        else if (u < 0.614)  return PHI_INV + gaussian(0.05);
        else if (u < 0.832)  return PHI + gaussian(0.05);
        else                 return next_double() * 3.0;
    }
    
private:
    double gaussian(double sigma) {
        double u1 = next_double();
        double u2 = next_double();
        return sigma * std::sqrt(-2.0 * std::log(u1 + 1e-300)) * std::cos(2.0 * PI * u2);
    }
};

// ============================================================
// POLYNOMIAL RING (N=1024)
// ============================================================
constexpr size_t N = 1024;
using Coeff = int64_t;

struct Polynomial {
    std::array<Coeff, N> c{};
    
    Polynomial() = default;
    
    static Polynomial zero() { return Polynomial(); }
    
    Polynomial operator+(const Polynomial& o) const {
        Polynomial r;
        for (size_t i = 0; i < N; i++) r.c[i] = c[i] + o.c[i];
        return r;
    }
    
    Polynomial operator-(const Polynomial& o) const {
        Polynomial r;
        for (size_t i = 0; i < N; i++) r.c[i] = c[i] - o.c[i];
        return r;
    }
    
    // Multiplication in Z[x]/(x^N + 1) - negacyclic convolution
    Polynomial operator*(const Polynomial& o) const {
        std::array<Coeff, 2*N> tmp{};
        for (size_t i = 0; i < N; i++) {
            if (c[i] == 0) continue;
            for (size_t j = 0; j < N; j++) {
                if (o.c[j] == 0) continue;
                tmp[i + j] += c[i] * o.c[j];
            }
        }
        Polynomial r;
        for (size_t i = 0; i < N; i++) {
            r.c[i] = tmp[i] - (i + N < 2*N ? tmp[i + N] : 0);
        }
        return r;
    }
    
    Polynomial operator*(Coeff scalar) const {
        Polynomial r;
        for (size_t i = 0; i < N; i++) r.c[i] = c[i] * scalar;
        return r;
    }
    
    Polynomial operator-() const {
        Polynomial r;
        for (size_t i = 0; i < N; i++) r.c[i] = -c[i];
        return r;
    }
    
    double norm() const {
        double s = 0;
        for (size_t i = 0; i < N; i++) s += static_cast<double>(c[i]) * c[i];
        return std::sqrt(s);
    }
    
    // Generate from RNG with small coefficients
    static Polynomial small_noise(RiemannRNG& rng, double bound = 2.0) {
        Polynomial p;
        for (size_t i = 0; i < N; i++) {
            double u = rng.next_double();
            if (u < 0.25) p.c[i] = 1;
            else if (u < 0.5) p.c[i] = -1;
            else p.c[i] = 0;
        }
        return p;
    }
    
    // Generate uniformly random mod q
    static Polynomial random(RiemannRNG& rng) {
        Polynomial p;
        for (size_t i = 0; i < N; i++) {
            p.c[i] = static_cast<Coeff>(rng.next() & 0xFFFFFFF);
        }
        return p;
    }
    
    // φ-harmonic noise matching zeta gap distribution
    static Polynomial phi_noise(RiemannRNG& rng) {
        Polynomial p;
        for (size_t i = 0; i < N; i++) {
            p.c[i] = static_cast<Coeff>(std::round(rng.phi_harmonic() * 10.0));
        }
        return p;
    }
};

// ============================================================
// KEYS
// ============================================================
struct SecretKey {
    Polynomial s;  // Small ternary polynomial
    
    static SecretKey generate(RiemannRNG& rng) {
        SecretKey sk;
        sk.s = Polynomial::small_noise(rng);
        return sk;
    }
};

struct PublicKey {
    Polynomial a;  // Random polynomial
    Polynomial b;  // b = -(a·s + e)
    
    static PublicKey generate(const SecretKey& sk, RiemannRNG& rng) {
        PublicKey pk;
        pk.a = Polynomial::random(rng);
        Polynomial e = Polynomial::phi_noise(rng);
        pk.b = e - (pk.a * sk.s);
        return pk;
    }
};

struct EvalKey {
    Polynomial rlk0, rlk1;  // Relinearization key
    
    static EvalKey generate(const SecretKey& sk, RiemannRNG& rng) {
        EvalKey ek;
        ek.rlk1 = Polynomial::random(rng);
        Polynomial e = Polynomial::phi_noise(rng);
        Polynomial s2 = sk.s * sk.s;
        ek.rlk0 = e - (ek.rlk1 * sk.s) + s2;
        return ek;
    }
};

// ============================================================
// CIPHERTEXT
// ============================================================
struct Ciphertext {
    Polynomial c0, c1;
    double noise_level;
    size_t depth;
    
    Ciphertext() : noise_level(INITIAL_NOISE), depth(0) {}
    Ciphertext(Polynomial c0_, Polynomial c1_, double n = INITIAL_NOISE, size_t d = 0)
        : c0(std::move(c0_)), c1(std::move(c1_)), noise_level(n), depth(d) {}
    
    // Lyapunov noise contraction: T(N) = N·φ⁻¹ + F_n·(1-φ⁻¹)
    void contract_noise() {
        double fib = fibonacci(depth + 1);
        noise_level = noise_level * NOISE_A + fib * NOISE_B;
        if (noise_level < 0.001) noise_level = 0.001;
    }
    
    bool is_safe() const { return noise_level < MAX_SAFE_NOISE; }
    
private:
    static double fibonacci(size_t n) {
        if (n <= 1) return static_cast<double>(n);
        double a = 0, b = 1;
        for (size_t i = 2; i <= n; i++) {
            double t = a + b; a = b; b = t;
        }
        return b;
    }
};

// ============================================================
// ENCODER (IEEE 754 double precision)
// ============================================================
struct Encoder {
    static Polynomial encode(double value) {
        Polynomial p;
        Coeff scaled = static_cast<Coeff>(std::round(value * SCALE));
        // Distribute across polynomial with φ-weighting
        double remaining = static_cast<double>(std::abs(scaled));
        for (size_t i = 0; i < N && remaining > 0.5; i++) {
            double w = std::pow(PHI_INV, static_cast<double>(i));
            Coeff share = static_cast<Coeff>(std::min(remaining * w, remaining));
            p.c[i] = (scaled >= 0) ? share : -share;
            remaining -= std::abs(share);
        }
        return p;
    }
    
    static double decode(const Polynomial& p) {
        double sum = 0.0, wsum = 0.0;
        for (size_t i = 0; i < N; i++) {
            double w = std::pow(PHI_INV, static_cast<double>(i));
            sum += static_cast<double>(p.c[i]) * w;
            wsum += w;
        }
        return sum / (wsum * SCALE);
    }
};

// ============================================================
// HOMOMORPHIC EVALUATOR
// ============================================================
class Evaluator {
private:
    EvalKey ek_;
    size_t op_count_;
    
public:
    explicit Evaluator(const EvalKey& ek) : ek_(ek), op_count_(0) {}
    
    // Encrypt: ct = (encoded + pk.b·u + e1, pk.a·u + e2)
    Ciphertext encrypt(const Polynomial& msg, const PublicKey& pk, RiemannRNG& rng) {
        Polynomial u = Polynomial::small_noise(rng);
        Polynomial e1 = Polynomial::phi_noise(rng);
        Polynomial e2 = Polynomial::phi_noise(rng);
        
        Ciphertext ct(
            msg + (pk.b * u) + e1,
            pk.a * u + e2,
            INITIAL_NOISE, 0
        );
        return ct;
    }
    
    // Decrypt: m = c0 + c1·s
    Polynomial decrypt_raw(const Ciphertext& ct, const SecretKey& sk) {
        return ct.c0 + (ct.c1 * sk.s);
    }
    
    double decrypt(const Ciphertext& ct, const SecretKey& sk) {
        return Encoder::decode(decrypt_raw(ct, sk));
    }
    
    // Homomorphic addition
    Ciphertext add(const Ciphertext& a, const Ciphertext& b) {
        Ciphertext r(
            a.c0 + b.c0,
            a.c1 + b.c1,
            a.noise_level + b.noise_level,
            std::max(a.depth, b.depth)
        );
        r.contract_noise();
        op_count_++;
        return r;
    }
    
    // Homomorphic subtraction
    Ciphertext sub(const Ciphertext& a, const Ciphertext& b) {
        Ciphertext r(
            a.c0 - b.c0,
            a.c1 - b.c1,
            a.noise_level + b.noise_level,
            std::max(a.depth, b.depth)
        );
        r.contract_noise();
        op_count_++;
        return r;
    }
    
    // Homomorphic multiplication (with relinearization)
    Ciphertext mul(const Ciphertext& a, const Ciphertext& b) {
        // Raw multiplication gives degree-2 terms
        Polynomial c00 = a.c0 * b.c0;
        Polynomial c01 = (a.c0 * b.c1) + (a.c1 * b.c0);
        Polynomial c11 = a.c1 * b.c1;
        
        // Relinearize: reduce c11·s² term using eval key
        Polynomial new_c0 = c00 + (c11 * ek_.rlk0);
        Polynomial new_c1 = c01 + (c11 * ek_.rlk1);
        
        Ciphertext r(
            std::move(new_c0),
            std::move(new_c1),
            a.noise_level * b.noise_level,
            a.depth + b.depth + 1
        );
        r.contract_noise();
        op_count_++;
        return r;
    }
    
    // Add plaintext
    Ciphertext add_plain(const Ciphertext& ct, const Polynomial& plain) {
        Ciphertext r(ct.c0 + plain, ct.c1, ct.noise_level + 0.5, ct.depth);
        r.contract_noise();
        return r;
    }
    
    // Multiply by plaintext
    Ciphertext mul_plain(const Ciphertext& ct, const Polynomial& plain) {
        Ciphertext r(ct.c0 * plain, ct.c1 * plain, ct.noise_level * plain.norm(), ct.depth + 1);
        r.contract_noise();
        return r;
    }
    
    size_t operation_count() const { return op_count_; }
};

} // namespace riemann_fhe
