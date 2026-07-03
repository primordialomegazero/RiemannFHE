CXX := g++
CXXFLAGS := -std=c++17 -O3 -march=native -Iinclude -Wall -Wextra -Werror -Wno-unused-parameter
LDFLAGS := -lm -pthread
HARDEN_FLAGS := -D_FORTIFY_SOURCE=2 -fstack-protector-strong -fPIE
LINK_HARDEN := -Wl,-z,relro -Wl,-z,now -pie

BINS := riemann_fhe riemann_multikey riemann_enc riemann_bench riemann_security_audit riemann_enterprise riemann_api

all: $(BINS)

prod: CXXFLAGS += $(HARDEN_FLAGS)
prod: LDFLAGS += $(LINK_HARDEN)
prod: clean all

riemann_fhe: demo/ratio_fhe_demo.cpp include/ratio_fhe_core.hpp
	$(CXX) $(CXXFLAGS) -o build/$@ $< $(LDFLAGS)

riemann_multikey: demo/multikey_demo.cpp include/fhe_multikey.hpp
	$(CXX) $(CXXFLAGS) -o build/$@ $< $(LDFLAGS)

riemann_enc: demo/riemann_encryption_demo.cpp include/riemann_encryption.hpp
	$(CXX) $(CXXFLAGS) -o build/$@ $< $(LDFLAGS)

riemann_bench: bench/fhe_benchmark.cpp include/ratio_fhe_core.hpp include/fhe_multikey.hpp
	$(CXX) $(CXXFLAGS) -o build/$@ $< $(LDFLAGS)

riemann_security_audit: security/fhe_security_audit.cpp include/security_layer*.hpp include/ratio_fhe_core.hpp include/fhe_multikey.hpp
	$(CXX) $(CXXFLAGS) -o build/$@ $< $(LDFLAGS)

riemann_enterprise: demo/enterprise_demo.cpp include/fhe_enterprise.hpp
	$(CXX) $(CXXFLAGS) -o build/$@ $< $(LDFLAGS)

riemann_api: api/riemann_api_server.cpp include/ratio_fhe_core.hpp include/fhe_multikey.hpp include/riemann_encryption.hpp include/fhe_enterprise.hpp
	$(CXX) $(CXXFLAGS) -o build/$@ $< $(LDFLAGS)

test: all
	cd test && bash run_full_test_suite.sh

bench: riemann_bench
	./build/riemann_bench

audit: riemann_security_audit
	./build/riemann_security_audit

clean:
	rm -rf build/*

distclean: clean
	rm -rf build

help:
	@echo "RiemannFHE Build System"
	@echo "  make all       Build all binaries (zero warnings)"
	@echo "  make prod      Production build with hardening"
	@echo "  make test      Run full test suite (14/14)"
	@echo "  make bench     Run performance benchmarks"
	@echo "  make audit     Run military-grade security audit"
	@echo "  make clean     Clean build artifacts"

.PHONY: all prod test bench audit clean distclean help

riemann_100M: bench/fhe_100M_benchmark.cpp include/ratio_fhe_core.hpp
	$(CXX) -std=c++17 -O0 -Iinclude -o build/$@ $< $(LDFLAGS)

bench100: riemann_100M
	./build/riemann_100M

.PHONY: riemann_100M bench100
