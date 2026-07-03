# Contributing to RiemannFHE

First off, thank you for considering contributing to RiemannFHE! 🎉

## Code of Conduct

This project and everyone participating in it is governed by the principle of **φ-harmony** — all contributions are valued, and all contributors are respected.

## How Can I Contribute?

### Reporting Bugs

Before creating bug reports, please check the existing issues. When you create a bug report, include:
- **Title**: Clear and descriptive
- **Description**: Exact steps to reproduce
- **Expected behavior**: What should happen
- **Actual behavior**: What actually happens
- **Environment**: OS, compiler version, CPU architecture
- **Minimal reproduction**: The smallest code snippet that demonstrates the issue

### Suggesting Enhancements

Enhancement suggestions are tracked as GitHub issues. Include:
- **Use case**: Why is this enhancement useful?
- **Proposed solution**: How should it work?
- **Alternatives considered**: What else did you think about?

### Pull Requests

1. Fork the repository
2. Create your feature branch: `git checkout -b feature/amazing-feature`
3. Commit your changes: `git commit -m 'Add amazing feature'`
4. Push to the branch: `git push origin feature/amazing-feature`
5. Open a Pull Request

### Pull Request Guidelines

- **Tests**: Add tests for new functionality
- **Documentation**: Update docs for changed behavior
- **Code style**: Follow existing patterns in the codebase
- **Commits**: Use clear, descriptive commit messages
- **One feature per PR**: Keep pull requests focused

## Development Setup

### Prerequisites
- **Compiler**: g++ 11+ or clang 14+
- **OS**: Linux (Ubuntu 22.04+), WSL2, or macOS
- **Build tools**: make, cmake (optional)

### Build from Source
```bash
git clone https://github.com/primordialomegazero/RiemannFHE.git
cd RiemannFHE
make all -j$(nproc)
```

### Run Tests
```bash
./run_full_test_suite.sh
```

### Run Benchmarks
```bash
./build/riemann_bench
```

### Run API Server (Demo Mode)
```bash
./build/riemann_api
```

## Project Structure

```
RiemannFHE/
├── ratio_fhe_core.hpp          # Core FHE engine (single-key)
├── fhe_multikey.hpp            # Multi-key FHE (Source + Flame Empress)
├── riemann_encryption.hpp      # Riemann Zeta encryption scheme
├── security_layer1.hpp         # Layer 1: Double φ Irrationality
├── security_layer2.hpp         # Layer 2: Anti-Polynomial
├── security_layer3.hpp         # Layer 3: Reverse Lattice
├── security_layer4.hpp         # Layer 4: φ-Harmonic Zeta Spectral
├── security_layer5.hpp         # Layer 5: Anti-LWE/RLWE
├── fhe_enterprise.hpp          # Enterprise hardening
├── fhe_benchmark.cpp           # Performance benchmarks
├── fhe_security_audit.cpp      # Military-grade security audit
├── riemann_api_server.cpp      # REST API server
├── run_full_test_suite.sh      # Automated test suite
├── Makefile                    # Build system
├── Dockerfile                  # Container build
├── THEOREMS.md                 # Formal proofs
├── API.md                      # API documentation
└── README.md                   # Main documentation
```

## Coding Standards

### C++ Style
- **Standard**: C++17
- **Indentation**: 4 spaces
- **Naming**: snake_case for variables/functions, PascalCase for classes
- **Comments**: Document all public APIs
- **Headers**: Use `#pragma once`

### Security Requirements
- **Constant-time**: All cryptographic operations must be constant-time
- **No undefined behavior**: Use sanitizers (-fsanitize=address,undefined)
- **Memory safety**: Zeroize sensitive data after use
- **No exceptions in crypto core**: Return error codes

## Testing

### Required Tests for New Features
- [ ] Unit tests for core functionality
- [ ] Integration tests with existing modules
- [ ] Security regression tests
- [ ] Performance benchmarks (before/after)

### Running Specific Tests
```bash
# Single-key FHE
./build/riemann_fhe

# Multi-key FHE
./build/riemann_multikey

# Security audit
./build/riemann_security_audit

# Enterprise hardening
./build/riemann_enterprise
```

## Questions?

Open an issue or contact the maintainer at:
- GitHub: [@primordialomegazero](https://github.com/primordialomegazero)
- Repository: [RiemannFHE](https://github.com/primordialomegazero/RiemannFHE)

---

*φΩ0 — Primordial Omega Zero*
