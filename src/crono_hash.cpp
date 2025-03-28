#include "../include/crono_hash.h"
#include "../include/crono_utils.h"
#include "../include/crono_math.h"
#include "../include/crono_quantum.h"
#include <sstream>
#include <iomanip>
#include <vector>    // <-- Hinzugefügt!

static std::string modeToString(CronoHash::CronoMode mode) {
    using CronoMode = CronoHash::CronoMode;
    switch (mode) {
    case CronoMode::FAST:      return "FAST";
    case CronoMode::BALANCED:  return "BALANCED";
    case CronoMode::SECURE:    return "SECURE";
    case CronoMode::ENTROPIC:  return "ENTROPIC";
    }
    return "UNKNOWN";
}

namespace CronoHash {

    std::string hash(const char* data, std::size_t length, double binding_duration_ms, CronoMode mode, unsigned int bit_strength) {
        // Anzahl der 64-Bit-Worte, die wir benötigen:
        unsigned int num_words = bit_strength / 64;
        if (num_words == 0) num_words = 1; // Absicherung

        // Array zur Speicherung – hier nutzen wir einen Vektor
        std::vector<uint64_t> words(num_words);

        // Für jeden Block einen anderen Startwert, z.B. indem wir tsc plus den Index einbeziehen
        uint64_t tsc = CronoUtils::get_tsc();
        uint64_t nano = CronoUtils::get_current_nanotime();
        uint64_t steady = CronoUtils::get_steady_time();
        uint64_t ram = CronoUtils::ram_fingerprint();
        uint64_t cache = CronoUtils::cache_noise();

        // Initialisierung der Worte – analog zur ersten Runde im Originalcode
        for (unsigned int i = 0; i < num_words; i++) {
            words[i] = tsc ^ (nano << ((i % 8) + 1)) ^ steady ^ ram ^ cache ^ CronoMath::PRIMES[i % CronoMath::NUM_PRIMES];
            words[i] = CronoUtils::mix_entropy(words[i], data, length);
            words[i] = CronoMath::endomorph_transform(words[i], tsc);
            words[i] = CronoMath::hash_const_mix(words[i]);
            words[i] = CronoMath::mod_prime(words[i]);
        }

        // Zweite Mischrunde
        for (unsigned int i = 0; i < num_words; i++) {
            words[i] = CronoMath::endomorph_transform(words[i], nano);
            words[i] = CronoUtils::mix_entropy(words[i], data, length);
            words[i] = CronoMath::hash_const_mix(words[i]);
            words[i] = CronoMath::mod_prime256(words[i], i % 4);
        }

        // Weitere Runden für SECURE/ENTROPIC
        if (mode == CronoMode::SECURE || mode == CronoMode::ENTROPIC) {
            for (unsigned int i = 0; i < num_words; i++) {
                words[i] = CronoMath::endomorph_transform(words[i], steady);
                words[i] = CronoUtils::mix_entropy(words[i], data, length);
            }
        }

        // ENTROPIC-Modus: zusätzlicher Memory Walk
        if (mode == CronoMode::ENTROPIC) {
            uint64_t mem_walk = CronoUtils::memory_walk();
            for (unsigned int i = 0; i < num_words; i++) {
                words[i] ^= mem_walk;
            }
        }

        // Adaptive Temp Binding
        if (binding_duration_ms > 0.0) {
            uint64_t binding_factor = CronoUtils::adaptive_binding_factor(binding_duration_ms);
            for (unsigned int i = 0; i < num_words; i++) {
                words[i] ^= binding_factor;
            }
        }

        // GhostSalt-Runde
        uint64_t gs = CronoUtils::ghost_salt();
        for (unsigned int i = 0; i < num_words; i++) {
            words[i] ^= gs;
        }

        // QUANTUM ROUND – erste Stufe via SHAKE128
        for (unsigned int i = 0; i < num_words; i++) {
            uint64_t qm = CronoQuantum::quantum_mix(words[i], data, length);
            words[i] ^= qm;
        }

        // QUANTUM ROUND – zweite Stufe via Kyber512
        for (unsigned int i = 0; i < num_words; i++) {
            uint64_t qm2 = CronoQuantum::quantum_mix_kyber(words[i], data, length);
            words[i] ^= qm2;
        }

        // Ausgabe als Hexadezimalstring – jeder 64-Bit Block wird zu 16 Hex-Zeichen
        std::ostringstream out;
        for (unsigned int i = 0; i < num_words; i++) {
            out << std::hex << std::setw(16) << std::setfill('0') << words[i];
        }
        return out.str();
    }

    std::string hash_with_metadata(const char* data, std::size_t length, double binding_duration_ms, CronoMode mode, unsigned int bit_strength) {
        uint64_t tsc = CronoUtils::get_tsc();
        uint64_t nano = CronoUtils::get_current_nanotime();
        uint64_t binding_factor = 0;
        if (binding_duration_ms > 0.0) {
            binding_factor = CronoUtils::adaptive_binding_factor(binding_duration_ms);
        }
        std::string hash_result = hash(data, length, binding_duration_ms, mode, bit_strength);
        std::ostringstream json;
        json << "{\n";
        json << "  \"hash\": \"" << hash_result << "\",\n";
        json << "  \"tsc\": " << tsc << ",\n";
        json << "  \"nano\": " << nano << ",\n";
        json << "  \"binding_factor\": \"0x" << std::hex << binding_factor << std::dec << "\",\n";
        json << "  \"mode\": \"" << modeToString(mode) << "\",\n";
        json << "  \"bit_strength\": " << bit_strength << "\n";
        json << "}";
        return json.str();
    }
}
