#pragma once
#include <cstddef>
#include <cstdint>

namespace CronoQuantum {
    // Bestehende Funktion
    uint64_t quantum_mix(uint64_t input, const char* data, std::size_t length);
    // Neue Funktion: Kyber512-Phase
    uint64_t quantum_mix_kyber(uint64_t input, const char* data, std::size_t length);
}
