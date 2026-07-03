#!/bin/bash
PASS=0; FAIL=0; TOTAL=0
BASE="http://localhost:8443"

# Start server in SERVE mode (background)
echo "Starting API server in serve mode..."
./build/riemann_api --serve &
SERVER_PID=$!
sleep 1

# Verify server is alive
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "❌ Server failed to start"
    exit 1
fi

test_endpoint() {
    TOTAL=$((TOTAL + 1))
    local name="$1"; local method="$2"; local url="$3"
    local data="$4"; local expected="$5"
    
    echo -n "  [$TOTAL] $name ... "
    
    if [ "$method" = "GET" ]; then
        response=$(curl -s --max-time 3 "$BASE$url" 2>/dev/null)
    else
        response=$(curl -s --max-time 3 -X "$method" "$BASE$url" \
            -H "Content-Type: application/json" -d "$data" 2>/dev/null)
    fi
    
    if echo "$response" | grep -q "$expected"; then
        echo "✅ PASS"
        PASS=$((PASS + 1))
    else
        echo "❌ FAIL"
        echo "       Expected: $expected"
        echo "       Got: ${response:0:120}"
        FAIL=$((FAIL + 1))
    fi
}

echo ""
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║  API INTEGRATION TESTS (Live Server)                        ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

# 1-10: All endpoints
test_endpoint "Health check"       "GET"  "/health"           ""                                    "healthy"
test_endpoint "Encrypt"            "POST" "/encrypt"           '{"value":42.0}'                      "success"
test_endpoint "Add"                "POST" "/add"               '{"a":15.0,"b":25.0}'                 "40"
test_endpoint "Multiply"           "POST" "/multiply"          '{"a":6.0,"b":7.0}'                   "42"
test_endpoint "Multi-Key Encrypt"  "POST" "/multi/encrypt"     '{"value":42.0}'                      "dual_key_security"
test_endpoint "Riemann Encrypt"    "POST" "/riemann/encrypt"   '{"value":42.0,"zero_index":15}'      "critical_point"
test_endpoint "Benchmark"          "GET"  "/benchmark"         ""                                    "encrypt_tps"
test_endpoint "Security Audit"     "GET"  "/audit"             ""                                    "MILITARY-GRADE"

# Stop server
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

echo ""
echo "══════════════════════════════════════════════════════"
echo "  Tests: $TOTAL | Pass: $PASS | Fail: $FAIL"
[ $FAIL -eq 0 ] && echo "  ✅ ALL API ENDPOINTS WORKING" || echo "  ❌ $FAIL FAILED"
echo "══════════════════════════════════════════════════════"
exit $FAIL
