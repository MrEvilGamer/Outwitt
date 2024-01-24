// outwitt.cpp

#include "memory.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <vector>
#include <sstream>

class MemoryScanner {
private:
    HANDLE processHandle;
    uintptr_t baseAddress;
    size_t bufferSize;
    std::vector<uint8_t> buffer;

public:
    MemoryScanner(HANDLE handle, uintptr_t baseAddr, size_t size)
        : processHandle(handle), baseAddress(baseAddr), bufferSize(size), buffer(size) {}

    // ... Other methods

    // Print memory region information
    void PrintMemoryRegionInfo() {
        MEMORY_BASIC_INFORMATION memInfo;
        VirtualQueryEx(processHandle, reinterpret_cast<LPCVOID>(baseAddress), &memInfo, sizeof(memInfo));
        std::cout << "Base address: 0x" << std::hex << memInfo.BaseAddress << std::endl;
        std::cout << "Region size: 0x" << std::hex << memInfo.RegionSize << std::endl;
        std::cout << "State: " << memInfo.State << std::endl;
        std::cout << "Protect: " << memInfo.Protect << std::endl;
        std::cout << "Type: " << memInfo.Type << std::endl;
    }

    // Find pattern in the buffer
    uintptr_t FindPattern(const std::vector<uint8_t>& pattern) {
        for (size_t i = 0; i < bufferSize - pattern.size(); ++i) {
            bool patternMatch = true;

            for (size_t j = 0; j < pattern.size(); ++j) {
                if (pattern[j] != 0x00 && pattern[j] != buffer[i + j]) {
                    patternMatch = false;
                    break;
                }
            }

            if (patternMatch) {
                // Calculate the absolute address
                return baseAddress + i;
            }
        }

        return 0;
    }
};



// Helper function to get process ID by name
DWORD GetProcessIdFromName(const char* processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &entry)) {
        do {
            if (strcmp(entry.szExeFile, processName) == 0) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return 0;
}

// Helper function to convert hex string to byte vector
std::vector<uint8_t> HexStringToBytes(const std::string& hex) {
    std::vector<uint8_t> bytes;

    for (size_t i = 0; i < hex.size(); i += 2) {
        // Skip '?' characters in the pattern
        if (hex[i] == '?' || hex[i + 1] == '?') {
            continue;
        }

        std::string byteString = hex.substr(i, 2);

        // Use std::istringstream for conversion without exceptions
        std::istringstream converter(byteString);
        int byteValue;

        // Check if the conversion was successful
        if (converter >> std::hex >> byteValue) {
            bytes.push_back(static_cast<uint8_t>(byteValue));
        }
        else {
            // Handle conversion failure (e.g., log an error)
            std::cerr << "Error converting hex string: " << byteString << std::endl;
        }
    }

    return bytes;
}

int main() {
    const char* targetProcessName = "SoTGame.exe"; // Replace with your process name
    const char* patternString = "48 8B 05 ?? ?? ?? ?? 48 8B 88 ?? ?? ?? ?? 48 85 C9 74 06 48 8B 49 70";

    DWORD processId = GetProcessIdFromName(targetProcessName);
    if (processId != 0) {
        std::cerr << "Process found." << std::endl;
        HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);

        if (processHandle != nullptr) {
            MODULEENTRY32 moduleEntry;
            moduleEntry.dwSize = sizeof(MODULEENTRY32);

            HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);

            uintptr_t moduleBase = 0;

            if (snapshot != INVALID_HANDLE_VALUE) {
                if (Module32First(snapshot, &moduleEntry)) {
                    moduleBase = reinterpret_cast<uintptr_t>(moduleEntry.modBaseAddr);
                }
                CloseHandle(snapshot);
            }

            MemoryScanner scanner(processHandle, moduleBase, 0x200000); // Increased buffer size to 2 MB
            if (scanner.ReadMemory()) {
                std::vector<uint8_t> pattern = MemoryScanner::HexStringToBytes(patternString);
                uintptr_t result = scanner.FindPattern(pattern);

                if (result != 0) {
                    std::cout << "Pattern found at address: 0x" << std::hex << result << std::endl;
                }
                else {
                    std::cout << "Pattern not found." << std::endl;
                }
            }
            else {
                std::cerr << "Error reading process memory." << std::endl;
            }

            CloseHandle(processHandle);
        }
        else {
            std::cerr << "Error opening process." << std::endl;
        }
    }
    else {
        std::cerr << "Process not found." << std::endl;
    }

    return 0;
}
