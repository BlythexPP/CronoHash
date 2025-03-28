// CronoHashTests.cpp
#include <gtest/gtest.h>
#include "../include/crono_hash.h"
#include <thread>
#include <chrono>
#include <iostream>

// Test: 128-Bit Hash im BALANCED-Modus
TEST(CronoHashTest, Hash128Balanced) {
    std::string input = "TestInput123";
    auto result = CronoHash::hash(input.c_str(), input.length(), 0, CronoHash::CronoMode::BALANCED, 128);
    // 128 Bit = 16 Bytes = 32 Hex-Zeichen
    EXPECT_EQ(result.length(), 32);
    std::cout << "[Hash128Balanced] Hash: " << result << std::endl;
}

// Test: 256-Bit Hash im SECURE-Modus mit Zeitbindung
TEST(CronoHashTest, Hash256Secure) {
    std::string input = "AnotherTestInput456";
    auto result = CronoHash::hash(input.c_str(), input.length(), 1000, CronoHash::CronoMode::SECURE, 256);
    // 256 Bit = 32 Bytes = 64 Hex-Zeichen
    EXPECT_EQ(result.length(), 64);
    std::cout << "[Hash256Secure] Hash: " << result << std::endl;
}

// Edge-Case: Minimaler Input (leerer String)
TEST(CronoHashTest, MinimalInput) {
    std::string input = "";
    auto result = CronoHash::hash(input.c_str(), input.length(), 0, CronoHash::CronoMode::FAST, 256);
    EXPECT_EQ(result.length(), 64);
    std::cout << "[MinimalInput] Hash: " << result << std::endl;
}

// Test: Zeitgebundene Hashes sollten sich bei gleichem Input unterscheiden
TEST(CronoHashTest, TimeBindingHashDiffers) {
    std::string input = "RepeatableInput";
    auto hash1 = CronoHash::hash(input.c_str(), input.length(), 1000, CronoHash::CronoMode::SECURE, 256);
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    auto hash2 = CronoHash::hash(input.c_str(), input.length(), 1000, CronoHash::CronoMode::SECURE, 256);
    EXPECT_NE(hash1, hash2);
    std::cout << "[TimeBindingHashDiffers] Hash1: " << hash1 << "\nHash2: " << hash2 << std::endl;
}

// Test: Ohne Zeitbindung sollten Hashes deterministisch sein
TEST(CronoHashTest, DeterministicNoBinding) {
    std::string input = "StableInput123";
    auto hash1 = CronoHash::hash(input.c_str(), input.length(), 0, CronoHash::CronoMode::FAST, 256);
    auto hash2 = CronoHash::hash(input.c_str(), input.length(), 0, CronoHash::CronoMode::FAST, 256);
    EXPECT_EQ(hash1, hash2);
    std::cout << "[DeterministicNoBinding] Hash: " << hash1 << std::endl;
}

// Test: Überprüfung der Metadaten-Ausgabe
TEST(CronoHashTest, MetadataCheck) {
    std::string input = "MetadataTest";
    auto metadata = CronoHash::hash_with_metadata(input.c_str(), input.length(), 500, CronoHash::CronoMode::ENTROPIC, 256);
    EXPECT_NE(metadata.find("\"mode\": \"ENTROPIC\""), std::string::npos);
    EXPECT_NE(metadata.find("\"bit_strength\": 256"), std::string::npos);
    std::cout << "[MetadataCheck] Metadata: " << metadata << std::endl;
}

// Test: Überprüfe verschiedene Bitstärken
TEST(CronoHashTest, VaryingBitStrengths) {
    std::string input = "BitStrengthTest";
    for (unsigned int bit = 128; bit <= 2048; bit *= 2) {
        auto hash = CronoHash::hash(input.c_str(), input.length(), 0, CronoHash::CronoMode::BALANCED, bit);
        size_t expectedLength = bit / 4; // 1 Hex-Zeichen = 4 Bit
        EXPECT_EQ(hash.length(), expectedLength) << "Fehler bei Bitstärke: " << bit;
        std::cout << "[VaryingBitStrengths] " << bit << "-Bit Hash: " << hash << std::endl;
    }
}

// Test: Überprüfe verschiedene Modi
TEST(CronoHashTest, DifferentModes) {
    std::string input = "ModeTest";
    CronoHash::CronoMode modes[] = { CronoHash::CronoMode::FAST, CronoHash::CronoMode::BALANCED,
                                     CronoHash::CronoMode::SECURE, CronoHash::CronoMode::ENTROPIC };
    for (auto mode : modes) {
        auto hash = CronoHash::hash(input.c_str(), input.length(), 500, mode, 256);
        EXPECT_EQ(hash.length(), 64);
        std::string modeStr;
        switch (mode) {
        case CronoHash::CronoMode::FAST: modeStr = "FAST"; break;
        case CronoHash::CronoMode::BALANCED: modeStr = "BALANCED"; break;
        case CronoHash::CronoMode::SECURE: modeStr = "SECURE"; break;
        case CronoHash::CronoMode::ENTROPIC: modeStr = "ENTROPIC"; break;
        }
        std::cout << "[DifferentModes] Mode " << modeStr << " Hash: " << hash << std::endl;
    }
}

