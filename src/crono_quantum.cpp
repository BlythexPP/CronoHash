#include "../include/crono_quantum.h"
#include <oqs/sha3.h>   // SHA3 Header von liboqs
#include <oqs/oqs.h>    // Allgemeine OQS-Funktionen
#include <oqs/kem.h>    // Für KEM-Funktionen und -Längen
#include <cstring>

namespace CronoQuantum {

    uint64_t quantum_mix(uint64_t input, const char* data, std::size_t length) {
        size_t total_len = sizeof(input) + length;
        unsigned char* buffer = new unsigned char[total_len];

        for (size_t i = 0; i < sizeof(input); i++) {
            buffer[i] = static_cast<unsigned char>((input >> (8 * i)) & 0xFF);
        }
        memcpy(buffer + sizeof(input), data, length);

        unsigned char output[32];
        OQS_SHA3_shake128(output, 32, buffer, total_len);
        delete[] buffer;

        uint64_t qm = 0;
        for (size_t i = 0; i < 8; i++) {
            qm |= static_cast<uint64_t>(output[i]) << (8 * i);
        }
        return qm;
    }

    uint64_t quantum_mix_kyber(uint64_t input, const char* data, std::size_t length) {
        // Erstelle einen kombinierten Puffer wie in quantum_mix
        size_t total_len = sizeof(input) + length;
        unsigned char* buffer = new unsigned char[total_len];
        for (size_t i = 0; i < sizeof(input); i++) {
            buffer[i] = static_cast<unsigned char>((input >> (8 * i)) & 0xFF);
        }
        memcpy(buffer + sizeof(input), data, length);

        // Nutze SHAKE128, um einen 64-Byte Seed zu erzeugen
        unsigned char shake_output[64];
        OQS_SHA3_shake128(shake_output, sizeof(shake_output), buffer, total_len);
        delete[] buffer;

        // Initialisiere Kyber512 KEM über den Algorithmusnamen
        OQS_KEM* kem = OQS_KEM_new("Kyber512");
        if (kem == nullptr) {
            // Fallback: interpretiere SHAKE-Ergebnis als uint64_t
            uint64_t fallback = 0;
            for (size_t i = 0; i < 8; i++) {
                fallback |= static_cast<uint64_t>(shake_output[i]) << (8 * i);
            }
            return fallback;
        }

        // Verwende die Membervariablen, die die Längen der jeweiligen Puffer enthalten
        size_t pk_len = kem->length_public_key;
        size_t ct_len = kem->length_ciphertext;
        size_t ss_len = kem->length_shared_secret;

        unsigned char* public_key = new unsigned char[pk_len];
        unsigned char* ciphertext = new unsigned char[ct_len];
        unsigned char* shared_secret = new unsigned char[ss_len];

        // Erzeuge einen zufälligen Public Key (nur für den Encaps-Aufruf benötigt)
        OQS_randombytes(public_key, pk_len);

        if (OQS_KEM_encaps(kem, ciphertext, shared_secret, public_key) != OQS_SUCCESS) {
            // Fehlerfall: Fallback
            uint64_t fallback = 0;
            for (size_t i = 0; i < 8; i++) {
                fallback |= static_cast<uint64_t>(shake_output[i]) << (8 * i);
            }
            OQS_KEM_free(kem);
            delete[] public_key;
            delete[] ciphertext;
            delete[] shared_secret;
            return fallback;
        }

        // Kombiniere das Shared Secret mit dem SHAKE-Seed via XOR zu einem 64-Bit Wert
        uint64_t qm2 = 0;
        for (size_t i = 0; i < 8 && i < ss_len; i++) {
            qm2 |= static_cast<uint64_t>(shared_secret[i] ^ shake_output[i]) << (8 * i);
        }

        OQS_KEM_free(kem);
        delete[] public_key;
        delete[] ciphertext;
        delete[] shared_secret;
        return qm2;
    }
}
