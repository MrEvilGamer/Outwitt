// memory.h
#ifndef MEMORY_H
#define MEMORY_H

#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <iostream>

bool IsProcessRunning(const char* processName);
DWORD GetProcessIdFromName(const char* processName);

class MemoryScanner {
private:
    HANDLE processHandle;
    uintptr_t baseAddress;
    size_t bufferSize;
    std::vector<uint8_t> buffer;

public:
    MemoryScanner(HANDLE handle, uintptr_t baseAddr, size_t size);

    bool ReadMemory();

    // Function to find pattern in the buffer
    uintptr_t FindPattern(const std::vector<uint8_t>& pattern);

    // Function to find UWorld offset
    uintptr_t FindUWorldOffset();

    // Function to find GObject offset
    uintptr_t FindGObjectOffset();

    // Function to find GName offset
    uintptr_t FindGNameOffset();
};

#endif // MEMORY_H
