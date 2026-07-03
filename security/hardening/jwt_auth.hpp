#pragma once
// ============================================================
// JWT Authentication Module (Adapted for RiemannFHE)
// HMAC-SHA256 JWT with φ-derived secret rotation
// φΩ0 — Primordial Omega Zero
// ============================================================
#include <string>
#include <vector>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <openssl/hmac.h>
#include <openssl/sha.h>

namespace riemann_auth {

constexpr double PHI = 1.61803398874989484820;
constexpr uint64_t JWT_EXPIRY = 3600;  // 1 hour

struct JWTToken {
    std::string header;
    std::string payload;
    std::string signature;
    
    std::string encode() const {
        return header + "." + payload + "." + signature;
    }
};

class JWTAuth {
private:
    std::string secret_;
    uint64_t created_at_;
    
    std::string base64_encode(const std::string& input) {
        static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string output;
        int val = 0, valb = -6;
        for (unsigned char c : input) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                output.push_back(chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) output.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
        while (output.size() % 4) output.push_back('=');
        return output;
    }
    
    std::string hmac_sha256(const std::string& data) {
        unsigned char result[SHA256_DIGEST_LENGTH];
        HMAC(EVP_sha256(), secret_.c_str(), secret_.size(),
             (const unsigned char*)data.c_str(), data.size(), result, nullptr);
        return std::string((char*)result, SHA256_DIGEST_LENGTH);
    }
    
public:
    JWTAuth(const std::string& secret) : secret_(secret) {
        created_at_ = std::chrono::system_clock::now().time_since_epoch().count();
    }
    
    JWTToken create_token(const std::string& user_id, const std::string& role = "user") {
        JWTToken token;
        
        // Header
        token.header = base64_encode(R"({"alg":"HS256","typ":"JWT"})");
        
        // Payload
        auto now = std::chrono::system_clock::now();
        auto exp = now + std::chrono::seconds(JWT_EXPIRY);
        auto exp_ts = std::chrono::duration_cast<std::chrono::seconds>(exp.time_since_epoch()).count();
        
        std::ostringstream payload;
        payload << "{"
                << "\"sub\":\"" << user_id << "\","
                << "\"role\":\"" << role << "\","
                << "\"iat\":" << std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() << ","
                << "\"exp\":" << exp_ts << ","
                << "\"phi\":" << PHI
                << "}";
        token.payload = base64_encode(payload.str());
        
        // Signature
        std::string message = token.header + "." + token.payload;
        token.signature = base64_encode(hmac_sha256(message));
        
        return token;
    }
    
    bool verify_token(const std::string& encoded_token) {
        size_t dot1 = encoded_token.find('.');
        size_t dot2 = encoded_token.find('.', dot1 + 1);
        if (dot1 == std::string::npos || dot2 == std::string::npos) return false;
        
        std::string header = encoded_token.substr(0, dot1);
        std::string payload = encoded_token.substr(dot1 + 1, dot2 - dot1 - 1);
        std::string signature = encoded_token.substr(dot2 + 1);
        
        std::string message = header + "." + payload;
        std::string expected_sig = base64_encode(hmac_sha256(message));
        
        return signature == expected_sig;
    }
    
    // φ-based secret rotation
    void rotate_secret() {
        uint64_t hash = 0;
        for (char c : secret_) hash = (hash * 31 + c) ^ (hash >> 27);
        secret_ = std::to_string(hash * (uint64_t)(PHI * 1e15));
    }
};

} // namespace riemann_auth
