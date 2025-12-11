#include "MemoryManager.h"

MemoryManager::MemoryManager(int totalSize) 
    : totalSize(totalSize), memory(totalSize, -1) 
{
    std::cout << "[Memoria] Inicializada con " << totalSize 
              << " bloques.\n";
}

bool MemoryManager::allocateMemoryFirstFit(int pid, int size) {
    int blocksAvailable = 0;
    int startIndex = -1;

    // Iterate through memory to find a suitable block
    for (int i = 0; i < totalSize; i++) {
        if (memory[i] == -1) {
            if (blocksAvailable == 0)
                startIndex = i;

            blocksAvailable++;

            if (blocksAvailable == size) {
                for (int j = startIndex; j < startIndex + size; j++) {
                    memory[j] = pid;
                }
                std::cout << "[Memoria] Asignar " << size 
                          << " bloques al PID=" << pid 
                          << " en el rango: [" << startIndex 
                          << ", " << startIndex + size - 1 << "]\n";
                return true;
            }
        } else {
            blocksAvailable = 0;
        }
    }

    std::cout << "[Memoria] ERROR: No hay suficiente memoria para PID = " 
              << pid << " (" << size << " bloques necesarios)\n";
    return false;
}

void MemoryManager::freeMemory(int pid) {
    for (int i = 0; i < totalSize; i++) {
        if (memory[i] == pid)
            memory[i] = -1;
    }
    std::cout << "[Memoria] Se ha liberado la memoria ocupada por el PID=" << pid << "\n";
}

void MemoryManager::printMemory() const {
    std::cout << "\n=== Mapa de Memoria ===\n";
    for (int i = 0; i < totalSize; i++) {
        if (memory[i] == -1)
            std::cout << "[ ]";
        else
            std::cout << "[" << memory[i] << "]";
    }
    std::cout << "\n";
}
