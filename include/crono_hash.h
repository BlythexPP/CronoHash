#pragma once
#include <string>
#include <cstddef>

namespace CronoHash {

    // Neue Config-Flags für unterschiedliche Hash-Modi
    enum class CronoMode {
        FAST,
        BALANCED,
        SECURE,
        ENTROPIC
    };

    // Liefert den 256-Bit Hash als hexadezimale Zeichenkette (64 Zeichen).
    // binding_duration_ms definiert die Temp-Binding-Dauer in Millisekunden.
    // mode steuert zusätzliche Mix-Runden und Extra-Entropie.
    // Neuer Parameter "bit_strength" (in Bit), z. B. 128, 256, 512, 1024, 2048.
    std::string hash(const char* data, std::size_t length, double binding_duration_ms = 0.0, CronoMode mode = CronoMode::BALANCED, unsigned int bit_strength = 256);

    std::string hash_with_metadata(const char* data, std::size_t length, double binding_duration_ms = 0.0, CronoMode mode = CronoMode::BALANCED, unsigned int bit_strength = 256);
}
