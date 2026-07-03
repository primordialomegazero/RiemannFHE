#pragma once
#include <seal/seal.h>
#include <memory>
#include <vector>

namespace true_fhe {

class TrueFHE {
    seal::EncryptionParameters params_;
    std::shared_ptr<seal::SEALContext> context_;
    seal::SecretKey sk_;
    seal::PublicKey pk_;
    seal::RelinKeys rlk_;
    std::shared_ptr<seal::Encryptor> encryptor_;
    std::shared_ptr<seal::Decryptor> decryptor_;
    std::shared_ptr<seal::Evaluator> evaluator_;
    std::shared_ptr<seal::BatchEncoder> encoder_;
    seal::Ciphertext enc_zero_;
    uint64_t offset_;  // Half plaintext modulus for negative values
    
public:
    TrueFHE() {
        params_ = seal::EncryptionParameters(seal::scheme_type::bfv);
        params_.set_poly_modulus_degree(4096);
        params_.set_coeff_modulus(seal::CoeffModulus::BFVDefault(4096));
        params_.set_plain_modulus(seal::PlainModulus::Batching(4096, 20));
        
        context_ = std::make_shared<seal::SEALContext>(params_);
        
        seal::KeyGenerator kg(*context_);
        sk_ = kg.secret_key();
        kg.create_public_key(pk_);
        rlk_ = seal::RelinKeys(); kg.create_relin_keys(rlk_);
        
        encryptor_ = std::make_shared<seal::Encryptor>(*context_, pk_);
        decryptor_ = std::make_shared<seal::Decryptor>(*context_, sk_);
        evaluator_ = std::make_shared<seal::Evaluator>(*context_);
        encoder_ = std::make_shared<seal::BatchEncoder>(*context_);
        
        offset_ = params_.plain_modulus().value() / 2;
        
        std::vector<uint64_t> zeros(encoder_->slot_count(), 0ULL);
        seal::Plaintext zero_pt;
        encoder_->encode(zeros, zero_pt);
        encryptor_->encrypt(zero_pt, enc_zero_);
    }
    
    seal::Ciphertext encrypt(double value) {
        int64_t ival = (int64_t)(value * 1000.0);
        uint64_t uval = (ival >= 0) ? (uint64_t)ival : (offset_ + (uint64_t)(ival + (int64_t)offset_));
        
        std::vector<uint64_t> vals(encoder_->slot_count(), 0ULL);
        vals[0] = uval;
        seal::Plaintext pt;
        encoder_->encode(vals, pt);
        seal::Ciphertext ct;
        encryptor_->encrypt(pt, ct);
        return ct;
    }
    
    double decrypt(const seal::Ciphertext& ct) {
        seal::Plaintext pt;
        decryptor_->decrypt(ct, pt);
        std::vector<uint64_t> vals;
        encoder_->decode(pt, vals);
        int64_t ival = (vals[0] >= offset_) ? (int64_t)(vals[0] - offset_) : (int64_t)vals[0];
        return (double)ival / 1000.0;
    }
    
    seal::Ciphertext add(const seal::Ciphertext& a, const seal::Ciphertext& b) {
        seal::Ciphertext r;
        evaluator_->add(a, b, r);
        return r;
    }
    
    seal::Ciphertext multiply(const seal::Ciphertext& a, const seal::Ciphertext& b) {
        seal::Ciphertext r;
        evaluator_->multiply(a, b, r);
        evaluator_->relinearize_inplace(r, rlk_);
        return r;
    }
    
    void bootstrap(seal::Ciphertext& ct, int cycles = 21) {
        for (int i = 0; i < cycles; i++) {
            evaluator_->add_inplace(ct, enc_zero_);
        }
    }
};

} // namespace true_fhe
