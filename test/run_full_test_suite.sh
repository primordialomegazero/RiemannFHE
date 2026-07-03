#!/bin/bash
PASS=0; FAIL=0; TESTS=0

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║  RIEMANNFHE FULL TEST SUITE v2.0                            ║"
echo "║  φΩ0 — Primordial Omega Zero                                ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

check() {
    TESTS=$((TESTS+1))
    echo -n "  [$TESTS] $1 ... "
    shift
    if "$@" >/dev/null 2>&1; then
        echo "✅ PASS"; PASS=$((PASS+1))
    else
        echo "❌ FAIL"; FAIL=$((FAIL+1))
    fi
}

# SUITE 1: Single-Key
echo "═══ SUITE 1: Single-Key FHE ═══"
check "Binary exists" test -x ../build/riemann_fhe
check "Encrypt/Decrypt works" ../build/riemann_fhe

# SUITE 2: Multi-Key
echo ""
echo "═══ SUITE 2: Multi-Key FHE ═══"
check "Binary exists" test -x ../build/riemann_multikey
check "Multi-key works" ../build/riemann_multikey

# SUITE 3: Homomorphic
echo ""
echo "═══ SUITE 3: Homomorphic Operations ═══"
check "Add passes" grep -q "True Add Passed: 7/7" <(../build/riemann_fhe 2>&1)
check "Mul passes" grep -q "Mul Passed: 5/5" <(../build/riemann_fhe 2>&1)

# SUITE 4: Security
echo ""
echo "═══ SUITE 4: Security Audit ═══"
check "Binary exists" test -x ../build/riemann_security_audit
check "Security score >90%" grep -q "Security Score.*9[0-9]" <(../build/riemann_security_audit 2>&1)

# SUITE 5: Enterprise
echo ""
echo "═══ SUITE 5: Enterprise Hardening ═══"
check "Binary exists" test -x ../build/riemann_enterprise
check "Constant-time OK" grep -q "ct_equal.*PASS" <(../build/riemann_enterprise 2>&1)
check "Thread safety OK" grep -q "Concurrent ops.*10/10" <(../build/riemann_enterprise 2>&1)

# SUITE 6: Riemann Encryption
echo ""
echo "═══ SUITE 6: Riemann Encryption ═══"
check "Binary exists" test -x ../build/riemann_enc
check "Encrypt/Decrypt works" ../build/riemann_enc
check "Tamper detection" grep -q "TAMPER DETECTED" <(../build/riemann_enc 2>&1)

# RESULTS
echo ""
echo "╔══════════════════════════════════════════════════════════════╗"
printf "║  TESTS: %-2d  |  PASS: %-2d  |  FAIL: %-2d  |  RATE: %3.0f%%     ║\n" $TESTS $PASS $FAIL $(echo "$PASS*100/$TESTS" | bc)
echo "╚══════════════════════════════════════════════════════════════╝"
[ $FAIL -eq 0 ] && echo "  ✅ PRODUCTION READY" || echo "  ⚠ NEEDS FIXES"
exit $FAIL
