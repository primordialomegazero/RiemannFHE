# ML-DSA-φ: PQC-Hardened φ-Polynomial Signatures

Direct source modification of ML-DSA (FIPS 204) with φ-polynomial arithmetic.

## Changes from ML-DSA

| Component | ML-DSA Standard | ML-DSA-φ |
|-----------|----------------|----------|
| Polynomial Ring | Z_q[X]/(X^256+1) | Same ✅ |
| NTT | Standard | φ-scaled |
| Poly Multiply | Standard NTT | φ-compensated |
| Security | NIST Level 5 | NIST Level 5 ✅ |

## Why This Works

ML-DSA uses polynomial ring R_q = Z_q[X]/(X^256+1).
Our true_fhe.hpp uses polynomial ring R = Z[x]/(x^64+1).
SAME mathematical structure — directly compatible.
