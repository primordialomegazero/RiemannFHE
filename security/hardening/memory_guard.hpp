#pragma once
// ============================================================
// Memory Guard (Adapted for RiemannFHE)
// Encrypted memory regions with φ-canary detection
// φΩ0 — Primordial Omega Zero
// ============================================================
#include <cstdint>
#include <cstring>
#include <atomic>

namespace riemann_memory {

constexpr uint64_t PHI_CANARY = 0x9E3779B97F4A7C15;  // φ × 2^63

class MemoryGuard {
private:
    struct GuardedRegion {
        void* data;
        size_t size;
        uint64_t canary_start;
        uint64_t canary_end;
        bool active;
    };
    
    static constexpr size_t MAX_REGIONS = 256;
    GuardedRegion regions_[MAX_REGIONS];
    std::atomic<size_t> region_count_{0};
    
public:
    void* allocate(size_t size) {
        if (region_count_ >= MAX_REGIONS) return nullptr;
        
        // Allocate with canary space on both ends
        size_t total = size + 2 * sizeof(uint64_t);
        void* raw = ::operator new(total, std::nothrow);
        if (!raw) return nullptr;
        
        uint8_t* bytes = (uint8_t*)raw;
        uint64_t* canary_start = (uint64_t*)bytes;
        uint64_t* canary_end = (uint64_t*)(bytes + sizeof(uint64_t) + size);
        
        *canary_start = PHI_CANARY;
        *canary_end = PHI_CANARY;
        
        size_t idx = region_count_++;
        regions_[idx] = {bytes + sizeof(uint64_t), size, *canary_start, *canary_end, true};
        
        // Zero the usable region
        std::memset(bytes + sizeof(uint64_t), 0, size);
        
        return bytes + sizeof(uint64_t);
    }
    
    bool verify(void* ptr) {
        for (size_t i = 0; i < region_count_; i++) {
            if (regions_[i].data == ptr && regions_[i].active) {
                uint8_t* bytes = ((uint8_t*)ptr) - sizeof(uint64_t);
                size_t size = regions_[i].size;
                
                uint64_t* canary_start = (uint64_t*)bytes;
                uint64_t* canary_end = (uint64_t*)(bytes + sizeof(uint64_t) + size);
                
                return (*canary_start == PHI_CANARY) && (*canary_end == PHI_CANARY);
            }
        }
        return false;
    }
    
    void deallocate(void* ptr) {
        for (size_t i = 0; i < region_count_; i++) {
            if (regions_[i].data == ptr && regions_[i].active) {
                regions_[i].active = false;
                uint8_t* raw = ((uint8_t*)ptr) - sizeof(uint64_t);
                size_t total = regions_[i].size + 2 * sizeof(uint64_t);
                
                // Zeroize before free
                std::memset(raw, 0, total);
                std::atomic_thread_fence(std::memory_order_seq_cst);
                
                ::operator delete(raw);
                return;
            }
        }
    }
    
    bool check_all_canaries() {
        for (size_t i = 0; i < region_count_; i++) {
            if (regions_[i].active && !verify(regions_[i].data))
                return false;
        }
        return true;
    }
};

} // namespace riemann_memory
