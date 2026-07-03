#include <iostream>
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <vector>

int main() {
    // Generate key
    EVP_PKEY_CTX* kctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
    EVP_PKEY_keygen_init(kctx);
    EVP_PKEY_CTX_set_ec_paramgen_curve_nid(kctx, NID_secp256k1);
    EVP_PKEY* pkey = nullptr;
    int ok = EVP_PKEY_generate(kctx, &pkey);
    EVP_PKEY_CTX_free(kctx);
    std::cout << "Keygen: " << (ok ? "OK" : "FAIL") << " pkey=" << (pkey ? "set" : "null") << "\n";
    
    std::string msg = "test message";
    
    // Sign
    EVP_MD_CTX* md = EVP_MD_CTX_new();
    EVP_PKEY_CTX* pctx = nullptr;
    ok = EVP_DigestSignInit(md, &pctx, EVP_sha256(), nullptr, pkey);
    std::cout << "SignInit: " << (ok == 1 ? "OK" : "FAIL") << "\n";
    
    size_t siglen = 0;
    ok = EVP_DigestSign(md, nullptr, &siglen, (const uint8_t*)msg.c_str(), msg.size());
    std::cout << "GetLen: " << (ok == 1 ? "OK" : "FAIL") << " len=" << siglen << "\n";
    
    std::vector<uint8_t> sig(siglen);
    ok = EVP_DigestSign(md, sig.data(), &siglen, (const uint8_t*)msg.c_str(), msg.size());
    std::cout << "Sign: " << (ok == 1 ? "OK" : "FAIL") << "\n";
    EVP_MD_CTX_free(md);
    
    // Verify
    md = EVP_MD_CTX_new();
    ok = EVP_DigestVerifyInit(md, nullptr, EVP_sha256(), nullptr, pkey);
    std::cout << "VerifyInit: " << (ok == 1 ? "OK" : "FAIL") << "\n";
    
    ok = EVP_DigestVerify(md, sig.data(), siglen, (const uint8_t*)msg.c_str(), msg.size());
    std::cout << "Verify: " << (ok == 1 ? "PASS" : "FAIL") << "\n";
    EVP_MD_CTX_free(md);
    
    EVP_PKEY_free(pkey);
    return 0;
}
