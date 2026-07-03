// Multi-recursive fractal multiply — scaled version
    FractalCiphertext recursive_mul(const FractalCiphertext& a, const FractalCiphertext& b, int depth) const {
        if (depth <= 0) return multiply(a, b);
        
        // Karatsuba-style but with φ-weighted scaling to prevent overflow
        FractalCiphertext a_high = a, a_low = a, b_high = b, b_low = b;
        double phi_factor = std::pow(PHI_INV, depth + 1);  // Extra damping
        
        for (int d = 0; d < MAX_FRACTAL_DEPTH; d++) {
            for (size_t i = 0; i < DIM; i++) {
                a_high.levels[d][i] *= phi_factor; 
                a_low.levels[d][i] *= (1.0 - phi_factor);
                b_high.levels[d][i] *= phi_factor; 
                b_low.levels[d][i] *= (1.0 - phi_factor);
            }
        }
        
        FractalCiphertext hh = recursive_mul(a_high, b_high, depth - 1);
        FractalCiphertext ll = recursive_mul(a_low, b_low, depth - 1);
        
        // Weighted combine (not full 4-product Karatsuba to avoid overflow)
        FractalCiphertext result = add(hh, ll);
        
        // Normalize by φ to compensate for recursive splitting
        for (int d = 0; d < MAX_FRACTAL_DEPTH; d++) {
            for (size_t i = 0; i < DIM; i++) {
                result.levels[d][i] *= PHI_INV;
            }
        }
        
        return result;
    }
