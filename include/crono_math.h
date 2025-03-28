#pragma once
#include <cstdint>

namespace CronoMath {
    // SHA256-ähnliche Primzahlen (für die erste Rundendurchläufe)
    extern const uint64_t* PRIMES;
    extern const int NUM_PRIMES;

    // Wendet zuerst den Modulo mit einer Basis-Primzahl an
    uint64_t mod_prime(uint64_t input);
    // Wendet einen zusätzlichen Modulo mit extra Primzahlen an (für 256-Bit)
    uint64_t mod_prime256(uint64_t input, int index);

    // Führt eine endomorphe Transformation durch (nichtlinear, seed-abhängig)
    uint64_t endomorph_transform(uint64_t input, uint64_t seed);
    // Mischt das Input mit konstanten Werten, um eine One-Way-Funktion zu erzeugen
    uint64_t hash_const_mix(uint64_t input);
}
