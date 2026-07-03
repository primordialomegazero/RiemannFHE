#include <iostream>
#include <vector>
#include <openssl/evp.h>
#include <openssl/ec.h>

int main() {
    // Generate key once
    EVP_PKEY_CTX* kctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
    EVP_PKEY_keygen_init(kctx);
    EVP_PKEY_CTX_set_ec_paramgen_curve_nid(kctx, NID_secp256k1);
    EVP_PKEY* pkey = nullptr;
    EVP_PKEY_generate(kctx, &pkey);
    EVP_PKEY_CTX_free(kctx);
    
    // Test 1: Sign then verify with SAME md context
    std::string msg = "test";
    
    // Sign
    EVP_MD_CTX* md = EVP_MD_CTX_new();
    EVP_PKEY_CTX* pctx = nullptr;
    EVP_DigestSignInit(md, &pctx, EVP_sha256(), nullptr, pkey);
    size_t len = 0;
    EVP_DigestSign(md, nullptr, &len, (const uint8_t*)msg.c_str(), msg.size());
    std::vector<uint8_t> sig(len);
    EVP_DigestSign(md, sig.data(), &len, (const uint8_t*)msg.c_str(), msg.size());
    EVP_MD_CTX_free(md);
    std::cout << "Sign: OK (" << len << " bytes)\n";
    
    // Verify with NEW context (like our class does)
    md = EVP_MD_CTX_new();
    EVP_DigestVerifyInit(md, nullptr, EVP_sha256(), nullptr, pkey);
    int r = EVP_DigestVerify(md, sig.data(), len, (const uint8_t*)msg.c_str(), msg.size());
    EVP_MD_CTX_free(md);
    std::cout << "Verify (new ctx): " << (r == 1 ? "PASS" : "FAIL") << "\n";
    
    // Verify with SAME signature buffer but different call
    md = EVP_MD_CTX_new();
    EVP_DigestVerifyInit(md, nullptr, EVP_sha256(), nullptr, pkey);
    r = EVP_DigestVerify(md, sig.data(), sig.size(), (const uint8_t*)msg.c_str(), msg.size());
    EVP_MD_CTX_free(md);
    std::cout << "Verify (sig.size): " << (r == 1 ? "PASS" : "FAIL") << "\n";
    
    EVP_PKEY_free(pkey);
    return 0;
}
