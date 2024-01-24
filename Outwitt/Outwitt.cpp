#include <iostream>
#include <Windows.h>
#include "memory.h"

int main() {
    const char* targetProcessName = "SoTGame.exe"; // Replace with your game's executable name

    if (IsProcessRunning(targetProcessName)) {
        std::cout << "SoT Process found." << std::endl;

        DWORD processId = GetProcessIdFromName(targetProcessName);
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

            MemoryScanner scanner(processHandle, moduleBase, 0x200000);
            if (scanner.ReadMemory()) {
                uintptr_t uWorldOffset = scanner.FindUWorldOffset();
                uintptr_t gObjectOffset = scanner.FindGObjectOffset();
                uintptr_t gNameOffset = scanner.FindGNameOffset();

                std::cout << "UWorld Offset: 0x" << std::hex << uWorldOffset << std::endl;
                std::cout << "GObject Offset: 0x" << std::hex << gObjectOffset << std::endl;
                std::cout << "GName Offset: 0x" << std::hex << gNameOffset << std::endl;
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
        std::cout << "SoT Process not found." << std::endl;
    }

    return 0;
}
