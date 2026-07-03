#pragma once
#include <cmath>
#include <vector>
#include <random>
#include <cstdint>

namespace true_fhe {

constexpr size_t POLY_N = 64;
constexpr int64_t Q = 65537;
constexpr int64_t Q_HALF = 32768;
constexpr double SIGMA = 2.0;
constexpr double SCALE = 100.0;

struct Polynomial {
    std::vector<int64_t> coeffs;
    Polynomial() : coeffs(POLY_N, 0) {}
    int64_t& operator[](size_t i) { return coeffs[i]; }
    const int64_t& operator[](size_t i) const { return coeffs[i]; }
    size_t size() const { return POLY_N; }
};

struct Ciphertext { Polynomial a, b; };

struct RelinKey { Polynomial a_rlk, b_rlk; };

class TrueFHE {
private:
    Polynomial sk_;
    RelinKey rlk_;
    uint64_t base_seed_;
    
    double gaussian(std::mt19937_64& rng) const {
        std::uniform_real_distribution<double> u(0,1);
        double u1=u(rng), u2=u(rng);
        return std::sqrt(-2.0*std::log(u1))*std::cos(2.0*M_PI*u2)*SIGMA;
    }
    
    Polynomial mul(const Polynomial& a, const Polynomial& b) const {
        Polynomial r;
        for(size_t i=0;i<POLY_N;i++) for(size_t j=0;j<POLY_N;j++){
            size_t k=(i+j)%POLY_N;
            int64_t s=((i+j)>=POLY_N)?-1:1;
            r[k]=(r[k]+s*a[i]*b[j])%Q;
        }
        for(size_t i=0;i<POLY_N;i++) r[i]=(r[i]+Q)%Q;
        return r;
    }
    
    Polynomial add(const Polynomial& a, const Polynomial& b) const {
        Polynomial r; for(size_t i=0;i<POLY_N;i++) r[i]=(a[i]+b[i])%Q;
        return r;
    }
    
    int64_t encode(double v) const {
        int64_t e=(int64_t)std::round(v*SCALE)%Q;
        return (e+Q)%Q;
    }
    
    double decode(int64_t v) const {
        if(v>Q_HALF) v-=Q;
        return (double)v/SCALE;
    }
    
    Ciphertext encrypt_poly(const Polynomial& m, uint64_t nonce) const {
        Ciphertext ct;
        std::mt19937_64 rng(base_seed_^nonce);
        std::uniform_int_distribution<int64_t> uniform(0,Q-1);
        for(size_t i=0;i<POLY_N;i++) ct.a[i]=uniform(rng);
        Polynomial as=mul(ct.a,sk_);
        for(size_t i=0;i<POLY_N;i++) ct.b[i]=(as[i]+(int64_t)std::round(gaussian(rng))+m[i])%Q;
        return ct;
    }
    
    Polynomial decrypt_poly(const Ciphertext& ct) const {
        Polynomial as=mul(ct.a,sk_), m;
        for(size_t i=0;i<POLY_N;i++) m[i]=(ct.b[i]-as[i]+Q)%Q;
        return m;
    }
    
public:
    TrueFHE(uint64_t seed=42):base_seed_(seed){
        std::mt19937_64 rng(seed);
        for(size_t i=0;i<POLY_N;i++){int r=rng()%3;sk_[i]=(r==0)?Q-1:(r==1)?0:1;}
        Polynomial s2=mul(sk_,sk_);
        Ciphertext rlk_ct=encrypt_poly(s2,99999);
        rlk_.a_rlk=rlk_ct.a; rlk_.b_rlk=rlk_ct.b;
    }
    
    Ciphertext encrypt(double value, uint64_t nonce=0) const {
        Polynomial m; m[0]=encode(value);
        return encrypt_poly(m,nonce);
    }
    
    double decrypt(const Ciphertext& ct) const {
        return decode(decrypt_poly(ct)[0]);
    }
    
    Ciphertext add(const Ciphertext& a, const Ciphertext& b) const {
        Ciphertext r; r.a=add(a.a,b.a); r.b=add(a.b,b.b); return r;
    }
    
    Ciphertext multiply(const Ciphertext& a, const Ciphertext& b) const {
        Polynomial c0=mul(a.a,b.a), c1=add(mul(a.a,b.b),mul(a.b,b.a)), c2=mul(a.b,b.b);
        Ciphertext r;
        r.a=add(c1,mul(c0,rlk_.a_rlk));
        r.b=add(c2,mul(c0,rlk_.b_rlk));
        return r;
    }
};

} // namespace true_fhe
