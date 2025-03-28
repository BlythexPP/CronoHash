#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00  // Windows 10 oder höher
#endif

#include "../include/crono_utils.h"
#include "../include/crono_math.h"  // Für endomorph_transform etc.
#include <chrono>
#include <cstdlib>
#include <thread>
#include <vector>
#include <algorithm>
#include <oqs/oqs.h>  // Für OQS_randombytes
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <intrin.h>
#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
#endif

namespace CronoUtils {

    // --- Bestehende Funktionen (unverändert) ---

    uint64_t get_current_nanotime() {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    }

    uint64_t get_tsc() {
        return __rdtsc();
    }

    uint64_t get_steady_time() {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    }

    uint64_t rotate_left(uint64_t value, int shift) {
        shift &= 63;
        return (value << shift) | (value >> (64 - shift));
    }

    uint64_t ram_fingerprint() {
        const size_t sample_size = 4096;
        unsigned char* sample = new unsigned char[sample_size];
        OQS_randombytes(sample, sample_size);
        uint64_t result = 0;
        for (size_t i = 0; i < sample_size; i++) {
            result ^= static_cast<uint64_t>(sample[i]) << (i % 8);
        }
        delete[] sample;
        return result;
    }

    uint64_t cache_noise() {
        volatile uint64_t sum = 0;
        int trials = 1000;
        for (int i = 0; i < trials; i++) {
            uint64_t t1 = __rdtsc();
            volatile int x = i * i;
            uint64_t t2 = __rdtsc();
            sum += (t2 - t1);
        }
        return sum ^ (sum << 7);
    }

    uint64_t mix_entropy(uint64_t seed, const char* data, std::size_t length) {
        for (std::size_t i = 0; i < length; i++) {
            seed ^= static_cast<uint64_t>(data[i]) << (i % 8);
            seed = rotate_left(seed, (i % 13) + 1);
            seed ^= ~((seed >> 17) | (seed << 47));
        }
        return seed;
    }

    uint64_t memory_walk() {
        const size_t arr_size = 1024;
        unsigned char buffer[arr_size];
        OQS_randombytes(buffer, arr_size);
        uint64_t walk = 0;
        for (size_t i = 0; i < arr_size; i += 7) {
            walk ^= static_cast<uint64_t>(buffer[i]) << (i % 8);
        }
        return walk;
    }

    static uint64_t thread_binding_factor(double duration_ms) {
        auto start = std::chrono::high_resolution_clock::now();
        uint64_t local_factor = 1;
        while (true) {
            auto now = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double, std::milli>(now - start).count();
            if (elapsed >= duration_ms)
                break;
            uint64_t sample_val = ram_fingerprint() ^ cache_noise();
            local_factor *= (sample_val | 1);
            for (volatile int j = 0; j < 50; j++) {}
            local_factor %= 0xFFFFFFFFFFFFFDULL;
        }
        return local_factor;
    }

    uint64_t adaptive_binding_factor(double duration_ms) {
        unsigned int num_threads = std::thread::hardware_concurrency();
        if (num_threads == 0)
            num_threads = 1;
        std::vector<uint64_t> results(num_threads, 1);
        std::vector<std::thread> threads;
        for (unsigned int i = 0; i < num_threads; i++) {
            threads.emplace_back([&results, i, duration_ms]() {
                results[i] = thread_binding_factor(duration_ms);
                });
        }
        for (auto& t : threads) {
            t.join();
        }
        uint64_t combined = 1;
        for (auto val : results) {
            combined *= (val | 1);
            combined %= 0xFFFFFFFFFFFFFDULL;
        }
        return combined;
    }

    uint64_t ghost_salt() {
        const size_t ghost_size = 2048;
        unsigned char* ghost = new unsigned char[ghost_size];
        OQS_randombytes(ghost, ghost_size);
        uint64_t salt = 0;
        for (size_t i = 0; i < ghost_size; i++) {
            salt ^= static_cast<uint64_t>(ghost[i]) << (i % 8);
        }
        delete[] ghost;
        return salt;
    }

#ifdef _WIN32
    // --- Windows-spezifische Hardware-Fingerprinting-Funktionen ---

    uint64_t get_cpu_id() {
        int cpuInfo[4] = { 0 };
        __cpuid(cpuInfo, 0);
        return (static_cast<uint64_t>(cpuInfo[0]) << 32) | static_cast<uint64_t>(cpuInfo[1]);
    }

    uint64_t tpm_fingerprint() {
        HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hr)) {
            return 0;
        }

        hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
            RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL, EOAC_NONE, NULL);

        if (FAILED(hr)) {
            CoUninitialize();
            return 0;
        }

        IWbemLocator* pLoc = NULL;
        hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
            IID_IWbemLocator, reinterpret_cast<LPVOID*>(&pLoc));
        if (FAILED(hr)) {
            CoUninitialize();
            return 0;
        }

        IWbemServices* pSvc = NULL;
        hr = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
        if (FAILED(hr)) {
            pLoc->Release();
            CoUninitialize();
            return 0;
        }

        hr = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
            RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
        if (FAILED(hr)) {
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return 0;
        }

        IEnumWbemClassObject* pEnumerator = NULL;
        hr = pSvc->ExecQuery(bstr_t("WQL"),
            bstr_t("SELECT ManufacturerID, ManufacturerVersion FROM Win32_Tpm"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL, &pEnumerator);

        uint64_t tpmFP = 0;
        if (SUCCEEDED(hr) && pEnumerator) {
            IWbemClassObject* pclsObj = NULL;
            ULONG uReturn = 0;
            if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK && uReturn != 0) {
                VARIANT vtProp;
                hr = pclsObj->Get(L"ManufacturerID", 0, &vtProp, 0, 0);
                if (SUCCEEDED(hr)) {
                    if (vtProp.vt == VT_UI4) {
                        tpmFP ^= vtProp.uintVal;
                    }
                }
                VariantClear(&vtProp);

                hr = pclsObj->Get(L"ManufacturerVersion", 0, &vtProp, 0, 0);
                if (SUCCEEDED(hr) && (vtProp.vt & VT_ARRAY)) {
                    SAFEARRAY* psa = vtProp.parray;
                    LONG lBound = 0, uBound = 0;
                    SafeArrayGetLBound(psa, 1, &lBound);
                    SafeArrayGetUBound(psa, 1, &uBound);
                    for (LONG i = lBound; i <= uBound; i++) {
                        BYTE byteVal = 0;
                        SafeArrayGetElement(psa, &i, &byteVal);
                        tpmFP ^= static_cast<uint64_t>(byteVal) << (i % 8);
                    }
                }
                VariantClear(&vtProp);
                pclsObj->Release();
            }
            pEnumerator->Release();
        }

        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return tpmFP;
    }

    uint64_t os_boot_time() {
        ULONGLONG uptime = GetTickCount64(); // in Millisekunden
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        ULARGE_INTEGER currentTime;
        currentTime.LowPart = ft.dwLowDateTime;
        currentTime.HighPart = ft.dwHighDateTime;
        uint64_t currentTimeNS = currentTime.QuadPart * 100; // FILETIME in 100-ns-Einheiten -> ns
        uint64_t uptimeNS = uptime * 1000000ULL;             // ms zu ns
        return currentTimeNS - uptimeNS;
    }

    uint64_t bios_serial() {
        HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hr)) {
            return 0;
        }

        hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
            RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL, EOAC_NONE, NULL);

        if (FAILED(hr)) {
            CoUninitialize();
            return 0;
        }

        IWbemLocator* pLoc = NULL;
        hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
            IID_IWbemLocator, reinterpret_cast<LPVOID*>(&pLoc));
        if (FAILED(hr)) {
            CoUninitialize();
            return 0;
        }

        IWbemServices* pSvc = NULL;
        hr = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
        if (FAILED(hr)) {
            pLoc->Release();
            CoUninitialize();
            return 0;
        }

        hr = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
            RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
        if (FAILED(hr)) {
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return 0;
        }

        IEnumWbemClassObject* pEnumerator = NULL;
        hr = pSvc->ExecQuery(bstr_t("WQL"),
            bstr_t("SELECT SerialNumber FROM Win32_BIOS"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL, &pEnumerator);

        uint64_t biosFP = 0;
        IWbemClassObject* pclsObj = NULL;
        ULONG uReturn = 0;
        if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK && uReturn != 0) {
            VARIANT vtProp;
            VariantInit(&vtProp);  // Initialisiere die VARIANT vor der Verwendung

            hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
            if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR && vtProp.bstrVal != NULL) {
                unsigned long hash = 5381;
                wchar_t* str = vtProp.bstrVal;
                while (*str) {
                    hash = ((hash << 5) + hash) + (*str);
                    str++;
                }
                biosFP = hash;
            }

            VariantClear(&vtProp);  // Bereinige die VARIANT
            pclsObj->Release();
        }

        pEnumerator->Release();
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();

        return biosFP;
    }

    // Verkettet alle Hardware-Fingerprints (ohne MAC) und mischt sie in 5 Runden mittels endomorph_transform
    std::string generate_system_chain_id() {
        uint64_t cpuID = get_cpu_id();
        uint64_t tpmFP = tpm_fingerprint();
        uint64_t bootTime = os_boot_time();
        uint64_t bios = bios_serial();
        uint64_t ramFP = ram_fingerprint();

        uint64_t chain = cpuID ^ tpmFP ^ bootTime ^ bios ^ ramFP;
        for (int i = 0; i < 5; i++) {
            chain = CronoMath::endomorph_transform(chain, get_tsc() ^ chain);
        }
        std::ostringstream oss;
        oss << std::hex << std::setw(16) << std::setfill('0') << chain;
        return oss.str();
    }

#endif  // _WIN32

} // namespace CronoUtils
