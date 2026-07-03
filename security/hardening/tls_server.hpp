#pragma once
// ============================================================
// TLS/HTTPS Server Module (Adapted for RiemannFHE)
// Production TLS 1.3 with φ-derived session keys
// φΩ0 — Primordial Omega Zero
// ============================================================
#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace riemann_tls {

constexpr double PHI = 1.61803398874989484820;

class TLSServer {
private:
    SSL_CTX* ctx_;
    int socket_fd_;
    bool initialized_;
    
public:
    TLSServer() : ctx_(nullptr), socket_fd_(-1), initialized_(false) {}
    
    bool initialize(const std::string& cert_path, const std::string& key_path) {
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        
        ctx_ = SSL_CTX_new(TLS_server_method());
        if (!ctx_) return false;
        
        // Enforce TLS 1.3 only
        SSL_CTX_set_min_proto_version(ctx_, TLS1_3_VERSION);
        SSL_CTX_set_max_proto_version(ctx_, TLS1_3_VERSION);
        
        // Load certificates
        if (SSL_CTX_use_certificate_file(ctx_, cert_path.c_str(), SSL_FILETYPE_PEM) <= 0)
            return false;
        if (SSL_CTX_use_PrivateKey_file(ctx_, key_path.c_str(), SSL_FILETYPE_PEM) <= 0)
            return false;
        
        // Security options
        SSL_CTX_set_options(ctx_, SSL_OP_ALL | SSL_OP_NO_RENEGOTIATION);
        
        initialized_ = true;
        return true;
    }
    
    // φ-derived session ID
    std::string generate_session_id(uint64_t seed) {
        uint64_t hash = (uint64_t)(seed * PHI * 1e15);
        char buf[33];
        snprintf(buf, sizeof(buf), "%016lx%016lx", hash, hash ^ 0xDEADBEEF);
        return std::string(buf, 32);
    }
    
    ~TLSServer() {
        if (ctx_) SSL_CTX_free(ctx_);
    }
};

} // namespace riemann_tls
