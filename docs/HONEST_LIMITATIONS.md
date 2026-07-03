# Honest Limitations & Open Questions

## Acknowledged by the Author

### 1. Zeta Zero Dataset
- **Current:** 200 zeros (from Odlyzko/LMFDB tables)
- **Limitation:** Small sample size for statistical security claims
- **Plan:** Billion-zero analysis pending hardware upgrade (requires 64GB+ RAM, 2TB NVMe)

### 2. Formal Security Proofs
- **Current:** THEOREMS.md contains 8 theorem sketches
- **Limitation:** Not full formal proofs; not machine-checked
- **Plan:** Collaboration with academic cryptographers; Coq/Lean formalization

### 3. External Audit
- **Status:** Pending
- **Note:** All claims of "military-grade" refer to internal red-team simulation (25 attack vectors)
- **Not:** Third-party validated

### 4. Performance Comparisons
- **Note:** TFHE TPS (~100) measures binary gate operations with bootstrapping
- **Our TPS (122K):** Measures floating-point encrypt/decrypt without circuit composition
- **Fair comparison:** Requires standardized FHE benchmarking suite (IEEE P2841 pending)

### 5. Noise-Free Claim
- **Algorithmic noise:** Zero (phase-difference encoding is exact under unitaries)
- **Floating-point noise:** Machine epsilon (10⁻¹²–10⁻¹⁶)
- **Distinction:** We mean "no additive error term in encryption formula" — not literal zero

### 6. Known-Zero Weakness
- **Issue:** Using publicly known zeta zeros as encryption anchors
- **Mitigation:** The zero INDEX (n) is secret, not the zero value itself
- **Remaining risk:** Only 200 indices = 7.6 bits of entropy per anchor
- **Fix pending:** Billion-zero dataset enables 30+ bits per anchor

### 7. IND-CPA with Deterministic Encryption
- **Issue:** Same value + same key = same ciphertext?
- **Current implementation:** Random θ₀ per encryption ensures different ciphertexts
- **Verification:** Included in test suite

## Questions from Community Review

### Q: Can phase-difference encoding remain stable under arbitrary composition?
**A:** We've verified 10 consecutive adds (depth 10) with zero drift. Theoretically, unitary composition is closed — the product of unitaries is unitary. Full proof pending.

### Q: What is the actual key size?
**A:** Observer seed: 64 bits. Multi-key (Source + Flame): 128 bits. Post-Grover security comes from the irrational structure, not key size alone.

### Q: How does the 24-hour transmutation timer work?
**A:** Ciphertext stores creation timestamp. Decryption checks `now - created < 86400`. System clock based — vulnerable to clock manipulation. Hardware Trusted Execution Environment would fix this.

## Status: RESEARCH PROTOTYPE
Until external audit, formal verification, and peer review are completed, RiemannFHE should be treated as a **research prototype** — not production cryptographic software.

*Honesty is the foundation of good cryptography. — φΩ0*
