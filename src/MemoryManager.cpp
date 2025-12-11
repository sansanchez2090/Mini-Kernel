#include "MemoryManager.h"

MemoryManager::MemoryManager(int totalSize) 
    : totalSize(totalSize), memory(totalSize, -1) 
{
    std::cout << "[Memoria] Inicializada con " << totalSize 
              << " bloques.\n";
}

bool MemoryManager::findAndAllocateFirstFit(int pid, int size, bool isPostCompaction) {
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
                std::cout << "[Memoria] Asignar " 
                          << (isPostCompaction ? "(despues de compactar) " : "")
                          << size << " bloques al PID=" << pid 
                          << " en el rango: [" << startIndex 
                          << ", " << startIndex + size - 1 << "]\n";
                return true;
            }
        } else {
            blocksAvailable = 0;
        }
    }

    return false;
}

bool MemoryManager::allocateMemoryFirstFit(int pid, int size) {
    
    // Try #1
    if (findAndAllocateFirstFit(pid, size, false)) {
        return true;
    }

    std::cout << "[Memoria] First-Fit fallo. Intentando compactacion...\n";

    compactMemory();

    // Try #2
    if (findAndAllocateFirstFit(pid, size, true)) {
        return true;
    }

    // If Try 2 fails
    std::cout << "[Memoria] ERROR: No hay suficiente memoria para PID = " 
              << pid << " (" << size << " bloques necesarios)\n";
    return false;
}

void MemoryManager::freeMemory(int pid) {

    std::cout << "\n[Memoria] Preparando para liberar memoria del PID=" << pid << "\n";
    printMemory();
    for (int i = 0; i < totalSize; i++) {
        if (memory[i] == pid)
            memory[i] = -1;
    }
    std::cout << "[Memoria] Se ha liberado la memoria ocupada por el PID=" << pid << "\n";
}

void MemoryManager::compactMemory() {
    std::cout << "\n[Memoria] INICIANDO COMPACTACION...\n";
    std::cout << ">>> Memoria ANTES de compactar (Fragmentada) <<<\n";
    printMemory();

    std::vector<int> newMemory(totalSize, -1);
    int writeIndex = 0;

    // Moved all blocks to the left
    for (int i = 0; i < totalSize; i++) {
        if (memory[i] != -1) {
            newMemory[writeIndex] = memory[i];
            writeIndex++;
        }
    }

    memory = newMemory;

    std::cout << "\n[Memoria] COMPACTACION FINZALIZADA...\n";
    std::cout << ">>> Memoria DESPUES de compactar (Contigua) <<<\n";
    printMemory();
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
