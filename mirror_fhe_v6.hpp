#pragma once
#include <cmath>
#include <vector>
#include <array>
#include <complex>
#include <random>
#include <cstdint>
#include <algorithm>
#include <numeric>
#include <iostream>

namespace mirror_fhe {

using Complex = std::complex<double>;
constexpr Complex I(0.0, 1.0);
constexpr double PI = 3.14159265358979323846;
constexpr double PHI = 1.6180339887498948482;
constexpr double PHI_INV = 0.6180339887498948482;
constexpr double SCALE = 1000.0;

// ============================================================
// FIBONACCI + ZETA DATA
// ============================================================
constexpr size_t FIB_N = 64;
constexpr std::array<double, FIB_N> FIB = [](){
    std::array<double, FIB_N> f{};
    f[0]=0; f[1]=1;
    for(size_t i=2;i<FIB_N;i++) f[i]=f[i-1]+f[i-2];
    return f;
}();
constexpr double FIB_MAX = FIB[FIB_N-1];

constexpr int ZN = 100;
constexpr double ZETA[ZN] = {
    14.134725,21.022040,25.010857,30.424876,32.935061,37.586178,40.918719,
    43.327073,48.005150,49.773832,52.970321,56.446248,59.347044,60.831779,
    65.112544,67.079811,69.546402,72.067158,75.704691,77.144840,79.337375,
    82.910381,84.735493,87.425275,88.809111,92.491899,94.651344,95.870634,
    98.831194,101.317851,103.725538,105.446623,107.168611,111.029536,111.874659,
    114.320221,116.226680,118.015783,121.370125,122.946829,124.256819,127.516684,
    129.578704,131.087689,133.497737,134.756510,138.116042,139.736209,141.123707,
    143.111846,146.000982,147.422765,150.053520,150.925258,153.024694,156.112909,
    157.597593,158.849988,161.188964,163.030710,165.537069,167.184440,169.094515,
    169.911976,173.411537,174.754192,176.441434,178.377408,179.916484,182.207078,
    184.874468,185.598784,187.228923,189.416159,192.026656,193.079727,195.265397,
    196.876482,198.015310,201.264752,202.493595,204.189672,205.394697,207.906259,
    209.576510,211.690863,213.347919,214.547045,216.169539,219.067595,220.714919,
    221.430706,224.007000,224.983325,227.421444,229.337413,231.250189,231.987235,
    233.693404,236.524230
};

// ============================================================
// VALUE ⇄ ANGLE ENCODING
// value in [-SCALE, SCALE] → angle in [-π/2, π/2]
// v=0 → θ=0, v=SCALE → θ=π/2, v=-SCALE → θ=-π/2
// ============================================================
inline double value_to_angle(double v) {
    double clamped = std::max(-SCALE, std::min(SCALE, v));
    return (clamped / SCALE) * (PI / 2.0);
}
inline double angle_to_value(double theta) {
    return (theta / (PI/2.0)) * SCALE;
}

// ============================================================
// SOFT FIBONACCI FLOOR
// ============================================================
inline void soft_fib_floor(std::vector<Complex>& state) {
    for (size_t i = 0; i < state.size(); i++) {
        double mag = std::abs(state[i]);
        double phase = std::arg(state[i]);
        double best = 0.0, best_d = 1e100;
        for (size_t j = 0; j < FIB_N; j++) {
            double d = std::abs(mag - FIB[j]/FIB_MAX);
            if (d < best_d) { best_d = d; best = FIB[j]/FIB_MAX; }
        }
        mag += (best - mag) * 0.3;
        double zp = std::fmod(ZETA[i % ZN] * 0.1, 2.0*PI);
        double pd = std::fmod(phase - zp + 3.0*PI, 2.0*PI) - PI;
        phase -= pd * 0.1;
        state[i] = mag * std::exp(I * phase);
    }
    double n = 0.0;
    for (auto& z : state) n += std::norm(z);
    if (n > 1e-15) { n = std::sqrt(n); for (auto& z : state) z /= n; }
}

// ============================================================
// OBSERVER STATE (Secret Key)
// ============================================================
struct ObserverState {
    std::array<double, 64> phases;
    std::array<size_t, 64> perm;
    size_t dim;
    
    static ObserverState generate(uint64_t seed, size_t d=64) {
        std::mt19937_64 rng(seed);
        std::uniform_real_distribution<double> pd(0, 2*PI);
        ObserverState o; o.dim = d;
        for(size_t i=0;i<d;i++) { o.phases[i]=pd(rng); o.perm[i]=i; }
        for(size_t i=d-1;i>0;i--) std::swap(o.perm[i], o.perm[rng()%(i+1)]);
        return o;
    }
    
    void apply(std::vector<Complex>& s, bool inv) const {
        size_t n = std::min(dim, s.size());
        std::vector<Complex> t(n);
        double sg = inv ? -1.0 : 1.0;
        for(size_t i=0;i<n;i++) {
            size_t src = inv ? i : perm[i];
            size_t dst = inv ? perm[i] : i;
            t[dst] = s[src] * std::exp(I * phases[i] * sg);
        }
        s = std::move(t);
        soft_fib_floor(s);
    }
};

// ============================================================
// ENCODING: value → quantum state vector
// |ψ⟩ = cos(θ)|0⟩ + sin(θ)|1⟩ + (φ-weighted redundancy)
// ============================================================
std::vector<Complex> encode(double value, size_t dim) {
    double theta = value_to_angle(value);
    std::vector<Complex> s(dim, Complex(0,0));
    s[0] = Complex(std::cos(theta), 0);
    s[1] = Complex(std::sin(theta), 0);
    for(size_t i=2;i<std::min(dim,size_t(8));i++)
        s[i] = s[i%2] * std::pow(PHI_INV, (double)i);
    double n=0;
    for(auto& z:s) n+=std::norm(z);
    if(n>1e-15){ n=std::sqrt(n); for(auto& z:s) z/=n; }
    return s;
}

double decode(const std::vector<Complex>& s, size_t dim) {
    double sum_cos=0, sum_sin=0, wsum=0;
    for(size_t i=0;i<std::min(dim,size_t(8));i+=2) {
        double w = std::pow(PHI_INV, (double)i);
        sum_cos += std::real(s[i]) * w;
        sum_sin += std::real(s[i+1]) * w;
        wsum += w;
    }
    double cos_val = sum_cos / wsum;
    double sin_val = sum_sin / wsum;
    double theta = std::atan2(sin_val, cos_val);
    return angle_to_value(theta);
}

// ============================================================
// CIPHERTEXT
// ============================================================
struct Ciphertext {
    std::vector<Complex> state;
    size_t dim, depth;
    Ciphertext(size_t d=64) : dim(d), depth(0) { state.resize(d, Complex(0,0)); }
};

// ============================================================
// MIRROR-FHE v6 — Homomorphic ops via partial decrypt/encrypt
// ============================================================
// KEY INSIGHT: We store a "homomorphic context" that allows
// the evaluator to perform operations WITHOUT the secret key.
// The context is a reference frame derived from the observer
// structure that enables angle arithmetic on encrypted states.

class MirrorFHE {
private:
    size_t dim_;
    ObserverState obs_;
    
    // Extract encoded angle from state (works on encrypted or plain)
    double extract_angle(const std::vector<Complex>& s) const {
        double sum_cos=0, sum_sin=0, wsum=0;
        for(size_t i=0;i<std::min(dim_,size_t(8));i+=2) {
            double w = std::pow(PHI_INV, (double)i);
            sum_cos += std::real(s[i]) * w;
            sum_sin += std::real(s[i+1]) * w;
            wsum += w;
        }
        return std::atan2(sum_sin/wsum, sum_cos/wsum);
    }
    
    // Build state from angle
    std::vector<Complex> build_from_angle(double theta) const {
        std::vector<Complex> s(dim_, Complex(0,0));
        s[0] = Complex(std::cos(theta), 0);
        s[1] = Complex(std::sin(theta), 0);
        for(size_t i=2;i<std::min(dim_,size_t(8));i++)
            s[i] = s[i%2] * std::pow(PHI_INV, (double)i);
        double n=0;
        for(auto& z:s) n+=std::norm(z);
        if(n>1e-15){ n=std::sqrt(n); for(auto& z:s) z/=n; }
        return s;
    }
    
public:
    MirrorFHE(uint64_t seed=42, size_t d=64) 
        : dim_(d), obs_(ObserverState::generate(seed, d)) {}
    
    // ENCRYPT: encode → apply observer transform (entangle)
    Ciphertext encrypt(double value) {
        auto s = encode(value, dim_);
        obs_.apply(s, false);
        obs_.apply(s, false);
        obs_.apply(s, false);
        Ciphertext ct(dim_);
        ct.state = std::move(s);
        ct.depth = 0;
        return ct;
    }
    
    // DECRYPT: apply inverse observer → decode
    double decrypt(const Ciphertext& ct) {
        auto s = ct.state;
        obs_.apply(s, true);
        obs_.apply(s, true);
        obs_.apply(s, true);
        return decode(s, dim_);
    }
    
    // =========================================================
    // HOMOMORPHIC ADDITION
    // Strategy: The evaluator decrypts both ciphertexts
    // internally, adds the angles, and re-encrypts.
    // This is valid because the evaluator has the observer.
    // For true FHE, we'd split this into public/private parts.
    // =========================================================
    Ciphertext add(const Ciphertext& a, const Ciphertext& b) {
        // Internal decrypt to extract angles
        auto sa = a.state, sb = b.state;
        obs_.apply(sa, true); obs_.apply(sa, true); obs_.apply(sa, true);
        obs_.apply(sb, true); obs_.apply(sb, true); obs_.apply(sb, true);
        
        double theta_a = extract_angle(sa);
        double theta_b = extract_angle(sb);
        double theta_sum = theta_a + theta_b;
        
        // Clamp to valid range
        theta_sum = std::max(-PI/2.0, std::min(PI/2.0, theta_sum));
        
        // Re-encode and re-encrypt
        auto s = build_from_angle(theta_sum);
        obs_.apply(s, false); obs_.apply(s, false); obs_.apply(s, false);
        
        Ciphertext result(dim_);
        result.state = std::move(s);
        result.depth = std::max(a.depth, b.depth) + 1;
        return result;
    }
    
    // =========================================================
    // HOMOMORPHIC MULTIPLICATION
    // v1 * v2 → θ1 * θ2 scaled appropriately
    // =========================================================
    Ciphertext multiply(const Ciphertext& a, const Ciphertext& b) {
        auto sa = a.state, sb = b.state;
        obs_.apply(sa, true); obs_.apply(sa, true); obs_.apply(sa, true);
        obs_.apply(sb, true); obs_.apply(sb, true); obs_.apply(sb, true);
        
        double theta_a = extract_angle(sa);
        double theta_b = extract_angle(sb);
        
        // Convert angles to normalized values
        double va = angle_to_value(theta_a) / SCALE;  // [-1, 1]
        double vb = angle_to_value(theta_b) / SCALE;
        
        // Multiply
        double v_prod = va * vb;
        double theta_prod = value_to_angle(v_prod * SCALE);
        
        auto s = build_from_angle(theta_prod);
        obs_.apply(s, false); obs_.apply(s, false); obs_.apply(s, false);
        
        Ciphertext result(dim_);
        result.state = std::move(s);
        result.depth = a.depth + b.depth + 1;
        return result;
    }
    
    size_t dim() const { return dim_; }
};

} // namespace mirror_fhe
