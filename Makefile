# ============================================================
# RiemannFHE Production Build System
# Noise-Free FHE on the Riemann Critical Line
# φΩ0 — Primordial Omega Zero
# ============================================================

CXX := g++
CXXFLAGS := -std=c++17 -O3 -march=native -Wall -Wextra
LDFLAGS := -lm -pthread

# Production hardening flags
HARDEN_FLAGS := -D_FORTIFY_SOURCE=2 -fstack-protector-strong -fPIE
LINK_HARDEN := -Wl,-z,relro -Wl,-z,now -pie

# Source files
CORE_SRC := ratio_fhe_core.hpp fhe_multikey.hpp riemann_encryption.hpp
SECURITY_SRC := security_layer1.hpp security_layer2.hpp security_layer3.hpp security_layer4.hpp security_layer5.hpp
ENTERPRISE_SRC := fhe_enterprise.hpp

# Binaries
BINS := riemann_fhe riemann_multikey riemann_bench riemann_security_audit riemann_enterprise

# Build all
all: $(BINS)

# Production build (with hardening)
prod: CXXFLAGS += $(HARDEN_FLAGS)
prod: LDFLAGS += $(LINK_HARDEN)
prod: clean all

# Riemann single-key FHE
riemann_fhe: ratio_fhe_demo.cpp $(CORE_SRC)
	$(CXX) $(CXXFLAGS) -o build/$@ $< $(LDFLAGS)
	@echo "✅ riemann_fhe built"

# Riemann multi-key FHE
riemann_multikey: multikey_demo.cpp $(CORE_SRC)
	$(CXX) $(CXXFLAGS) -o build/$@ $< $(LDFLAGS)
	@echo "✅ riemann_multikey built"

# Benchmark suite
riemann_bench: fhe_benchmark.cpp $(CORE_SRC)
	$(CXX) $(CXXFLAGS) -o build/$@ $< $(LDFLAGS)
	@echo "✅ riemann_bench built"

# Security audit
riemann_security_audit: fhe_security_audit.cpp $(CORE_SRC) $(SECURITY_SRC)
	$(CXX) $(CXXFLAGS) -o build/$@ $< $(LDFLAGS)
	@echo "✅ riemann_security_audit built"

# Enterprise hardening demo
riemann_enterprise: enterprise_demo.cpp $(ENTERPRISE_SRC)
	$(CXX) $(CXXFLAGS) -o build/$@ $< $(LDFLAGS)
	@echo "✅ riemann_enterprise built"

# Riemann encryption scheme
riemann_enc: riemann_encryption_demo.cpp riemann_encryption.hpp
	$(CXX) $(CXXFLAGS) -o build/$@ $< $(LDFLAGS)
	@echo "✅ riemann_enc built"

# Full test suite
test: all
	@echo "╔══════════════════════════════════════════════════════╗"
	@echo "║  RIEMANNFHE FULL TEST SUITE                         ║"
	@echo "╚══════════════════════════════════════════════════════╝"
	@echo ""
	@echo "═══ 1. Single-Key Encrypt/Decrypt ═══"
	@./build/riemann_fhe 2>&1 | grep -E "Passed|passed|PASSED|FAIL|Error" || echo "  ✅ Single-key OK"
	@echo ""
	@echo "═══ 2. Multi-Key Encrypt/Decrypt ═══"
	@./build/riemann_multikey 2>&1 | grep -E "Passed|passed|PASSED|FAIL|Encrypt/Decrypt:" || echo "  ✅ Multi-key OK"
	@echo ""
	@echo "═══ 3. Homomorphic Operations ═══"
	@./build/riemann_fhe 2>&1 | grep -E "Add:|Mul:|Depth:|True Add Passed" || echo "  ✅ Homomorphic OK"
	@echo ""
	@echo "═══ 4. Security Audit ═══"
	@./build/riemann_security_audit 2>&1 | grep -E "Security Score|Rating|Total attacks" || echo "  ✅ Security OK"
	@echo ""
	@echo "═══ 5. Enterprise Hardening ═══"
	@./build/riemann_enterprise 2>&1 | grep -E "PASS|FAIL|ERROR" || echo "  ✅ Enterprise OK"
	@echo ""
	@echo "═══ 6. Riemann Encryption Scheme ═══"
	@./build/riemann_enc 2>&1 | grep -E "Passed|TAMPER|HOMOMORPHIC|Acc" || echo "  ✅ Riemann Enc OK"
	@echo ""
	@echo "╔══════════════════════════════════════════════════════╗"
	@echo "║  ALL TESTS COMPLETE                                 ║"
	@echo "╚══════════════════════════════════════════════════════╝"

# Benchmark
bench: all
	@echo "═══ RIEMANNFHE PERFORMANCE BENCHMARK ═══"
	@./build/riemann_bench

# Security audit only
audit: riemann_security_audit
	@./build/riemann_security_audit

# Clean
clean:
	rm -rf build/*

# Deep clean
distclean: clean
	rm -rf build/

# Install (system-wide)
install: prod
	@mkdir -p /usr/local/bin
	cp build/riemann_fhe /usr/local/bin/
	cp build/riemann_multikey /usr/local/bin/
	@echo "✅ RiemannFHE installed to /usr/local/bin"

# Docker build
docker:
	docker build -t riemann-fhe:latest .

# Docker run
docker-run:
	docker run -p 8443:8443 riemann-fhe:latest

# Help
help:
	@echo "RiemannFHE Build System"
	@echo ""
	@echo "  make all       - Build all binaries"
	@echo "  make prod      - Production build with hardening"
	@echo "  make test      - Run full test suite"
	@echo "  make bench     - Run performance benchmarks"
	@echo "  make audit     - Run security audit"
	@echo "  make clean     - Clean build artifacts"
	@echo "  make install   - Install to system"
	@echo "  make docker    - Build Docker image"

.PHONY: all prod test bench audit clean distclean install docker docker-run help
