#pragma once
#include <cstdint>
#include <cstddef>
#include <string>

namespace CronoUtils {

    // Bestehende Funktionen…
    uint64_t get_current_nanotime();
    uint64_t get_tsc();
    uint64_t get_steady_time();
    uint64_t rotate_left(uint64_t value, int shift);
    uint64_t ram_fingerprint();
    uint64_t cache_noise();
    uint64_t mix_entropy(uint64_t seed, const char* data, std::size_t length);
    uint64_t memory_walk();
    uint64_t adaptive_binding_factor(double duration_ms);
    uint64_t ghost_salt();

#ifdef _WIN32
    // Produktionsreife Hardware-Fingerprinting-Funktionen (Windows-spezifisch)
    uint64_t get_cpu_id();
    uint64_t tpm_fingerprint();
    uint64_t os_boot_time();
    uint64_t bios_serial();
    std::string generate_system_chain_id();
#endif

} // namespace CronoUtils
