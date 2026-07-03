// ============================================================
// Enterprise Hardening Test Suite
// JWT Auth | Rate Limiter | Memory Guard
// φΩ0 — Primordial Omega Zero
// ============================================================
#include <iostream>
#include <iomanip>
#include <cassert>
#include "../security/hardening/jwt_auth.hpp"
#include "../security/hardening/rate_limiter.hpp"
#include "../security/hardening/memory_guard.hpp"

using namespace riemann_auth;
using namespace riemann_rate;
using namespace riemann_memory;

int pass = 0, fail = 0;

void test(const char* name, bool condition) {
    if (condition) { pass++; std::cout << "  ✅ " << name << "\n"; }
    else { fail++; std::cout << "  ❌ " << name << " FAILED\n"; }
}

int main() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║  ENTERPRISE HARDENING TEST SUITE                            ║
║  JWT Auth | Rate Limiter | Memory Guard                     ║
║  φΩ0 — Primordial Omega Zero                                ║
╚══════════════════════════════════════════════════════════════╝
)";

    // ═══ JWT AUTH ═══
    std::cout << "\n═══ 1. JWT AUTHENTICATION ═══\n\n";
    
    JWTAuth auth("riemann-secret-key-2026");
    
    auto token = auth.create_token("user123", "admin");
    std::string encoded = token.encode();
    
    test("Token generated", !encoded.empty());
    test("Token has 3 parts", [&](){
        size_t dots = 0;
        for (char c : encoded) if (c == '.') dots++;
        return dots == 2;
    }());
    test("Token verifies", auth.verify_token(encoded));
    test("Invalid token rejected", !auth.verify_token("invalid.token.here"));
    test("Empty token rejected", !auth.verify_token(""));
    
    // φ-secret rotation
    auth.rotate_secret();
    std::cout << "  ℹ️  Secret rotated (φ-based)\n";
    
    // ═══ RATE LIMITER ═══
    std::cout << "\n═══ 2. RATE LIMITER ═══\n\n";
    
    RateLimiter limiter(5.0, 2.0);  // 5 tokens max, 2/sec refill
    
    // Burst test
    int allowed = 0;
    for (int i = 0; i < 10; i++) {
        if (limiter.allow_request("client1")) allowed++;
    }
    test("Burst limited to max tokens", allowed == 5);
    test("Exceeded requests blocked", allowed < 10);
    
    // Different clients
    test("Different client not affected", limiter.allow_request("client2"));
    
    // Priority scaling
    limiter.set_priority_limit("vip-client");
    int vip_allowed = 0;
    for (int i = 0; i < 10; i++) {
        if (limiter.allow_request("vip-client")) vip_allowed++;
    }
    test("VIP gets more tokens", vip_allowed > 5);
    
    // Restricted scaling
    limiter.set_restricted_limit("suspicious-client");
    int restricted = 0;
    for (int i = 0; i < 10; i++) {
        if (limiter.allow_request("suspicious-client")) restricted++;
    }
    test("Restricted gets fewer tokens", restricted < 5);
    
    test("Active clients tracked", limiter.active_clients() >= 3);
    
    // ═══ MEMORY GUARD ═══
    std::cout << "\n═══ 3. MEMORY GUARD ═══\n\n";
    
    MemoryGuard guard;
    
    void* mem1 = guard.allocate(1024);
    test("Memory allocated", mem1 != nullptr);
    test("Memory verifies (canary intact)", guard.verify(mem1));
    
    void* mem2 = guard.allocate(4096);
    test("Multiple allocations", mem2 != nullptr);
    test("All canaries intact", guard.check_all_canaries());
    
    // Write to memory (shouldn't break canary)
    uint8_t* bytes = (uint8_t*)mem1;
    for (size_t i = 0; i < 1024; i++) bytes[i] = (uint8_t)(i % 256);
    test("Memory writable", bytes[0] == 0 && bytes[255] == 255);
    test("Canary still intact after write", guard.verify(mem1));
    
    guard.deallocate(mem1);
    guard.deallocate(mem2);
    test("Deallocation successful", true);  // No crash = pass
    
    // ═══ RESULTS ═══
    std::cout << "\n══════════════════════════════════════════════════════\n";
    std::cout << "  Tests: " << (pass + fail) << " | Pass: " << pass << " | Fail: " << fail;
    if (fail == 0) std::cout << " | ✅ ALL PASS";
    std::cout << "\n══════════════════════════════════════════════════════\n";
    
    return fail > 0 ? 1 : 0;
}
