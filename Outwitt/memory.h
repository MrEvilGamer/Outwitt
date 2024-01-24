// memory.h
#ifndef MEMORY_H
#define MEMORY_H

#include <Windows.h>
#include <vector>
#include <string>

class MemoryScanner {
private:
    HANDLE processHandle;
    uintptr_t baseAddress;
    size_t bufferSize;
    std::vector<uint8_t> buffer;

public:
    MemoryScanner(HANDLE handle, uintptr_t baseAddr, size_t size);

    // Read memory into the buffer
    bool ReadMemory();

    // ... other member functions ...

    // Helper function to check if a process is running
    static bool IsProcessRunning(const char* processName);

    // Helper function to convert hex string to byte vector
    static std::vector<uint8_t> HexStringToBytes(const std::string& hex);
};

#endif // MEMORY_H
