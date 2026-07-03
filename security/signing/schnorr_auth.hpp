#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <openssl/evp.h>
#include <openssl/ec.h>

namespace riemann_schnorr {

constexpr double PHI = 1.61803398874989484820;

struct PhiHash {
    static uint64_t hash(const void* data, size_t len) {
        const uint8_t* bytes = (const uint8_t*)data;
        uint64_t h = 0x9E3779B97F4A7C15;
        for (size_t i = 0; i < len; i++) {
            h = (h ^ bytes[i]) * (uint64_t)(PHI * 1e15);
            h = (h << 7) | (h >> 57);
        }
        h ^= h >> 33; h *= 0xFF51AFD7ED558CCD; h ^= h >> 33;
        return h;
    }
};

class SchnorrSigner {
private:
    EVP_PKEY* pkey_;
public:
    SchnorrSigner() : pkey_(nullptr) {
        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
        EVP_PKEY_keygen_init(ctx);
        EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx, NID_secp256k1);
        EVP_PKEY_generate(ctx, &pkey_);
        EVP_PKEY_CTX_free(ctx);
    }
    ~SchnorrSigner() { if (pkey_) EVP_PKEY_free(pkey_); }
    
    std::vector<uint8_t> sign(const std::string& msg) const {
        EVP_MD_CTX* md = EVP_MD_CTX_new();
        EVP_PKEY_CTX* pctx = nullptr;
        EVP_DigestSignInit(md, &pctx, EVP_sha256(), nullptr, pkey_);
        
        size_t len = 0;
        EVP_DigestSign(md, nullptr, &len, (const uint8_t*)msg.c_str(), msg.size());
        std::vector<uint8_t> sig(len);
        EVP_DigestSign(md, sig.data(), &len, (const uint8_t*)msg.c_str(), msg.size());
        sig.resize(len);  // CRITICAL: resize to actual length
        EVP_MD_CTX_free(md);
        return sig;
    }
    
    bool verify(const std::string& msg, const std::vector<uint8_t>& sig) const {
        EVP_MD_CTX* md = EVP_MD_CTX_new();
        EVP_DigestVerifyInit(md, nullptr, EVP_sha256(), nullptr, pkey_);
        int r = EVP_DigestVerify(md, sig.data(), sig.size(),
                                  (const uint8_t*)msg.c_str(), msg.size());
        EVP_MD_CTX_free(md);
        return r == 1;
    }
};

} // namespace riemann_schnorr
