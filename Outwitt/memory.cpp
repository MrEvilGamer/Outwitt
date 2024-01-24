// memory.cpp

#include "memory.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <vector>
#include <sstream>

MemoryScanner::MemoryScanner(HANDLE handle, uintptr_t baseAddr, size_t size)
    : processHandle(handle), baseAddress(baseAddr), bufferSize(size), buffer(size) {}

bool MemoryScanner::ReadMemory() {
    return ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(baseAddress), buffer.data(), bufferSize, nullptr) != 0;
}

bool MemoryScanner::IsProcessRunning(const char* processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Error creating process snapshot." << std::endl;
        return false;
    }

    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &processEntry)) {
        do {
            if (_stricmp(processEntry.szExeFile, processName) == 0) {
                CloseHandle(snapshot);
                return true; // Process found
            }
        } while (Process32Next(snapshot, &processEntry));
    }

    CloseHandle(snapshot);
    return false; // Process not found
}

std::vector<uint8_t> MemoryScanner::HexStringToBytes(const std::string& hex) {
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
