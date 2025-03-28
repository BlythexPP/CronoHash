#include <iostream>
#include <cstring>
#include <string>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <ctime>
#include "include/crono_hash.h"
#include <oqs/sha3.h> // Für die Generierung eines sicheren Strings

// Verhindert Konflikte mit den Windows-Makros min/max
#define NOMINMAX
#ifdef _WIN32
#include <windows.h>
#endif

// Setzt die Position und Größe des Konsolenfensters (x, y, Breite, Höhe)
void set_console_window(int x, int y, int width, int height) {
#ifdef _WIN32
    HWND consoleWindow = GetConsoleWindow();
    if (consoleWindow) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD bufferSize;
        bufferSize.X = static_cast<SHORT>(width);
        bufferSize.Y = static_cast<SHORT>(height);
        SetConsoleScreenBufferSize(hConsole, bufferSize);
        MoveWindow(consoleWindow, x, y, width, height, TRUE);
    }
#endif
}


// Druckt das ASCII-Logo und Entwicklerdaten
void print_logo_and_developer_info() {
    std::cout << R"delim(
 a88888b.                                     dP     dP                    dP       
d8'   `88                                     88     88                    88       
88        88d888b. .d8888b. 88d888b. .d8888b. 88aaaaa88a .d8888b. .d8888b. 88d888b. 
88        88'  `88 88'  `88 88'  `88 88'  `88 88     88  88'  `88 Y8ooooo. 88'  `88 
Y8.   .88 88       88.  .88 88    88 88.  .88 88     88  88.  .88       88 88    88 
 Y88888P' dP       `88888P' dP    dP `88888P' dP     dP  `88888P8 `88888P' dP    dP 
                                                                                    
                                                                                                                                                                                                                                   
                                                                                                                                                                         
                                                                                                                                                                         
 








                                                                                                                                                                        
                                                                                                                                                                         
                                                                                                                                                                         
)delim" << std::endl;

    std::cout << "CronoHash v1.0" << std::endl;
    std::cout << "Entwickelt von Blythex" << std::endl;
    std::cout << "GitHub: https://github.com/BlythexPP" << std::endl;
    std::cout << "Datum: " << __DATE__ << "\n\n";
}

// Druckt die Hilfemeldung für Kommandozeilenparameter
void print_usage() {
    std::cout << "Usage: CronoHash [-i input_string] [-d binding_duration_ms] [-m mode]\n";
    std::cout << "  -i : Input string to hash (default: \"CronoHash Prime Core v1\")\n";
    std::cout << "  -d : Temp binding duration in milliseconds (default: 0, no binding)\n";
    std::cout << "  -m : Mode (FAST, BALANCED, SECURE, ENTROPIC) (default: BALANCED)\n";
    std::cout << "  -h : Show this help message\n";
}

// Interaktives Menü zur Auswahl der Temp-Binding-Dauer
double prompt_binding_duration() {
    std::cout << "Wählen Sie die Temp-Binding Option:\n";
    std::cout << "1. Einmaliger Login-Token (500 - 2000 ms)\n";
    std::cout << "2. RAM-only Secrets / Anti-Debug (50 - 200 ms)\n";
    std::cout << "3. Temporärer File-Decryption Key (3000 - 5000 ms)\n";
    std::cout << "4. Forensische Marker (5000 - 10000 ms)\n";
    std::cout << "5. Testing/Debug-Modus (60000 ms)\n";
    std::cout << "6. Custom\n";
    std::cout << "Ihre Wahl (1-6): ";

    int choice;
    std::cin >> choice;
    double duration_ms = 0;
    switch (choice) {
    case 1: duration_ms = 1000; break;
    case 2: duration_ms = 100; break;
    case 3: duration_ms = 4000; break;
    case 4: duration_ms = 7500; break;
    case 5: duration_ms = 60000; break;
    case 6:
        std::cout << "Geben Sie die gewünschte Dauer in Millisekunden ein: ";
        std::cin >> duration_ms;
        break;
    default:
        std::cout << "Ungültige Auswahl. Standardwert 0 wird verwendet.\n";
        duration_ms = 0;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return duration_ms;
}

// Interaktives Menü zur Auswahl des Modes
CronoHash::CronoMode prompt_mode() {
    std::cout << "Wählen Sie den Mode:\n";
    std::cout << "1. FAST\n";
    std::cout << "2. BALANCED\n";
    std::cout << "3. SECURE\n";
    std::cout << "4. ENTROPIC\n";
    std::cout << "Ihre Wahl (1-4): ";

    int choice;
    std::cin >> choice;
    CronoHash::CronoMode mode = CronoHash::CronoMode::BALANCED;
    switch (choice) {
    case 1: mode = CronoHash::CronoMode::FAST; break;
    case 2: mode = CronoHash::CronoMode::BALANCED; break;
    case 3: mode = CronoHash::CronoMode::SECURE; break;
    case 4: mode = CronoHash::CronoMode::ENTROPIC; break;
    default:
        std::cout << "Ungültige Auswahl. Standard BALANCED wird verwendet.\n";
        mode = CronoHash::CronoMode::BALANCED;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return mode;
}

// Generiert einen sicheren Zufallsstring mithilfe von liboqs SHAKE128.
// Der generierte String wird in Hexadezimaldarstellung zurückgegeben.
std::string generate_secure_string(size_t length) {
    unsigned char output[32];
    uint64_t seed = static_cast<uint64_t>(std::time(nullptr));
    unsigned char seed_bytes[8];
    for (size_t i = 0; i < 8; i++) {
        seed_bytes[i] = static_cast<unsigned char>((seed >> (8 * i)) & 0xFF);
    }
    OQS_SHA3_shake128(output, 32, seed_bytes, 8);
    std::ostringstream oss;
    size_t bytes_to_use = (length < 32 ? length : 32);
    for (size_t i = 0; i < bytes_to_use; i++) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)output[i];
    }
    return oss.str();
}

// Neue Hilfsfunktion zur Auswahl der Bitstärke:
unsigned int prompt_bit_strength() {
    std::cout << "Wählen Sie die gewünschte Bitstärke:\n";
    std::cout << "1. 128 Bit\n";
    std::cout << "2. 256 Bit\n";
    std::cout << "3. 512 Bit\n";
    std::cout << "4. 1024 Bit\n";
    std::cout << "5. 2048 Bit\n";
    std::cout << "Ihre Wahl (1-5): ";
    int choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    switch (choice) {
    case 1: return 128;
    case 2: return 256;
    case 3: return 512;
    case 4: return 1024;
    case 5: return 2048;
    default:
        std::cout << "Ungültige Auswahl. Standard 256 Bit wird verwendet.\n";
        return 256;
    }
}

int main(int argc, char* argv[]) {
    // Konsole positionieren etc.
    set_console_window(1066, 825, 1805, 873);
    print_logo_and_developer_info();

    std::string input = "CronoHash Prime Core v1";
    double binding_duration = 0.0;
    CronoHash::CronoMode mode = CronoHash::CronoMode::BALANCED;
    unsigned int bit_strength = 256;
    const size_t MIN_LENGTH = 8;

    // Kommandozeilenparameter verarbeiten (Ergänzung: "-b" für Bitstärke)
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (std::strcmp(argv[i], "-i") == 0 && (i + 1) < argc) {
                input = argv[++i];
            }
            else if (std::strcmp(argv[i], "-d") == 0 && (i + 1) < argc) {
                binding_duration = std::atof(argv[++i]);
            }
            else if (std::strcmp(argv[i], "-m") == 0 && (i + 1) < argc) {
                std::string modeStr = argv[++i];
                if (modeStr == "FAST")
                    mode = CronoHash::CronoMode::FAST;
                else if (modeStr == "BALANCED")
                    mode = CronoHash::CronoMode::BALANCED;
                else if (modeStr == "SECURE")
                    mode = CronoHash::CronoMode::SECURE;
                else if (modeStr == "ENTROPIC")
                    mode = CronoHash::CronoMode::ENTROPIC;
                else {
                    std::cout << "Unbekannter Mode. Standard BALANCED wird verwendet.\n";
                    mode = CronoHash::CronoMode::BALANCED;
                }
            }
            else if (std::strcmp(argv[i], "-b") == 0 && (i + 1) < argc) {
                bit_strength = std::atoi(argv[++i]);
                // Validierung: Erlaubt sind 128,256,512,1024,2048
                if (bit_strength != 128 && bit_strength != 256 && bit_strength != 512 &&
                    bit_strength != 1024 && bit_strength != 2048) {
                    std::cout << "Ungültige Bitstärke. Standard 256 Bit wird verwendet.\n";
                    bit_strength = 256;
                }
            }
            else if (std::strcmp(argv[i], "-h") == 0) {
                std::cout << "Usage: CronoHash [-i input_string] [-d binding_duration_ms] [-m mode] [-b bit_strength]\n";
                return 0;
            }
            else {
                std::cout << "Ungültiger Parameter.\n";
                std::cout << "Usage: CronoHash [-i input_string] [-d binding_duration_ms] [-m mode] [-b bit_strength]\n";
                return 1;
            }
        }
    }
    else {
        std::cout << "Geben Sie den Input-String ein (mindestens " << MIN_LENGTH << " Zeichen): ";
        std::getline(std::cin, input);
        if (input.empty()) {
            std::cout << "Kein Input eingegeben. Standardwert wird verwendet.\n";
            input = "CronoHash Prime Core v1";
        }
        binding_duration = prompt_binding_duration();
        mode = prompt_mode();
        bit_strength = prompt_bit_strength();
    }

    if (input.length() < MIN_LENGTH) {
        std::cout << "Input zu kurz. Generiere sicheren Input-String...\n";
        input = generate_secure_string(MIN_LENGTH);
        std::cout << "Generierter Input: " << input << "\n";
    }

    std::string hash = CronoHash::hash(input.c_str(), input.length(), binding_duration, mode, bit_strength);
    std::cout << bit_strength << "-Bit Hash: " << hash << std::endl;
    std::string meta = CronoHash::hash_with_metadata(input.c_str(), input.length(), binding_duration, mode, bit_strength);
    std::cout << "Metadaten: " << meta << std::endl;

    return 0;
}

