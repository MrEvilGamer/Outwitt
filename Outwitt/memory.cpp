// memory.cpp
#include "memory.h"

MemoryScanner::MemoryScanner(HANDLE handle, uintptr_t baseAddr, size_t size)
    : processHandle(handle), baseAddress(baseAddr), bufferSize(size), buffer(size) {}

bool MemoryScanner::ReadMemory() {
    return ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(baseAddress), buffer.data(), bufferSize, nullptr) != 0;
}

bool IsProcessRunning(const char* processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &entry)) {
        do {
            if (_stricmp(entry.szExeFile, processName) == 0) {
                CloseHandle(snapshot);
                return true;
            }
        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return false;
}

DWORD GetProcessIdFromName(const char* processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &entry)) {
        do {
            if (_stricmp(entry.szExeFile, processName) == 0) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return 0;
}

uintptr_t MemoryScanner::FindPattern(const std::vector<uint8_t>& pattern) {
    for (size_t i = 0; i < bufferSize - pattern.size(); ++i) {
        bool patternMatch = true;

        for (size_t j = 0; j < pattern.size(); ++j) {
            if (pattern[j] != 0x00 && pattern[j] != buffer[i + j]) {
                patternMatch = false;
                break;
            }
        }

        if (patternMatch) {
            return baseAddress + i;
        }
    }

    return 0;
}

uintptr_t MemoryScanner::FindUWorldOffset() {
    std::vector<uint8_t> uWorldPattern = { 0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x88, 0x00, 0x00, 0x00, 0x00, 0x48, 0x85, 0xC9, 0x74, 0x06, 0x48, 0x8B, 0x49, 0x70 };
    return FindPattern(uWorldPattern);
}

uintptr_t MemoryScanner::FindGObjectOffset() {
    std::vector<uint8_t> gObjectPattern = { 0x89, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xDF, 0x48, 0x89, 0x5C, 0x24 };
    return FindPattern(gObjectPattern);
}

uintptr_t MemoryScanner::FindGNameOffset() {
    std::vector<uint8_t> gNamePattern = { 0x48, 0x8B, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x48, 0x85, 0xDB, 0x75, 0x00, 0xB9, 0x08, 0x04, 0x00, 0x00 };
    return FindPattern(gNamePattern);
}
