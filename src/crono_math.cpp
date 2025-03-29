#include "../include/crono_math.h"
#include "../include/crono_utils.h"  // notwendig für CronoUtils::rotate_left
#include <algorithm>
#include <array>
#include <random>
#include <chrono>

namespace CronoMath {

    // 64 hochwertige 64-Bit-Primzahlen als statisches Array
    const std::array<uint64_t, 64> STATIC_PRIMES = {
        0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL,
        0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
        0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL,
        0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
        0xd807aa98a3030242ULL, 0x12835b0145706fbeULL,
        0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
        0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL,
        0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
        0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL,
        0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
        0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL,
        0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
        0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL,
        0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
        0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL,
        0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
        0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL,
        0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
        0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL,
        0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
        0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL,
        0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
        0xd192e819d6ef5218ULL, 0xd69906245565a910ULL,
        0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
        0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL,
        0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
        0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL,
        0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
        0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL,
        0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL
    };

    // Erzeuge einen modifizierbaren Vektor, der zur Laufzeit zufällig permutiert wird.
    std::array<uint64_t, 64> randomizedPrimes = STATIC_PRIMES;

    // Einmalige Initialisierung der zufälligen Permutation
    static void initializeRandomizedPrimes() {
        unsigned seed = static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
        std::shuffle(randomizedPrimes.begin(), randomizedPrimes.end(), std::default_random_engine(seed));
    }

    // Statische Initialisierung: Wird einmal beim Laden des Moduls ausgeführt
    static bool isInitialized = []() {
        initializeRandomizedPrimes();
        return true;
        }();

    // Definition der externen Variablen aus dem Header:
    const uint64_t* PRIMES = randomizedPrimes.data();
    const int NUM_PRIMES = static_cast<int>(randomizedPrimes.size());

    // Die restlichen Funktionen:
    uint64_t mod_prime(uint64_t input) {
        size_t index = input % randomizedPrimes.size();
        uint64_t divisor = randomizedPrimes[index];
        if (divisor == 0) {
            // Fehlerbehandlung oder Fallback-Wert
            divisor = 1; // Damit nicht durch 0 geteilt wird
        }
        return input % divisor;
    }


    uint64_t mod_prime256(uint64_t input, int index) {
        static const uint64_t extra_primes[8] = {
            0xCbbb9d5dc1059ed8ULL, 0x629a292a367cd507ULL,
            0x9159015a3070dd17ULL, 0x152fecd8f70e5939ULL,
            0x7F4A7C15E0F1A7B3ULL, 0xABCD12345678EF99ULL,
            0x99FF00AA11335577ULL, 0xCAFEBABEDEADCAFEULL
        };
        uint64_t rotated = CronoUtils::rotate_left(input, (index * 11) % 64);
        return rotated % extra_primes[index % 8];
    }


    static const uint64_t SECP_CONSTANT = 0xD1B54A32D192ED03ULL;

    uint64_t endomorph_transform(uint64_t input, uint64_t seed) {
        input ^= seed;
        input *= SECP_CONSTANT;
        input = CronoUtils::rotate_left(input, 13);
        input ^= seed;
        input *= SECP_CONSTANT;
        return input;
    }

    uint64_t hash_const_mix(uint64_t input) {
        const uint64_t salt1 = 0xA5A5A5A5A5A5A5A5ULL;
        const uint64_t salt2 = 0xDEADBEEF1337BEEFULL;
        const uint64_t salt3 = 0xC0FFEE1234567890ULL;
        const uint64_t rot = CronoUtils::rotate_left(input ^ salt1, 17);
        const uint64_t mix = (rot ^ salt2) * salt3;
        return mix ^ (mix >> 31);
    }

} // namespace CronoMath
