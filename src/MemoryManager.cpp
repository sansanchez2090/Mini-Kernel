#include "MemoryManager.h"

#include <algorithm>

MemoryManager::MemoryManager(int totalSize) 
    : totalSize(totalSize), memory(totalSize, -1) 
{
    // Log the initialization so tests know how many blocks are available.
    std::cout << "[Memoria] Inicializada con " << totalSize 
              << " bloques.\n";
}

bool MemoryManager::findAndAllocateFirstFit(int pid, int size, bool isPostCompaction, int* allocatedStart) {
    // Scan the linear memory to locate the first contiguous free window.
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
                if (allocatedStart) {
                    *allocatedStart = startIndex;
                }
                // Emit diagnostic info to compare pre/post-compaction placements.
                std::cout << "[Memoria] Asignar " 
                          << (isPostCompaction ? "(despues de compactar) " : "")
                          << size << " bloques al PID=" << pid 
                          << " en el rango: [" << startIndex 
                          << ", " << startIndex + size - 1 << "]\n";
                return true;
            }
        } else {
            // Reset the counter whenever an occupied frame appears.
            blocksAvailable = 0;
        }
    }

    return false;
}

bool MemoryManager::allocateMemoryFirstFit(int pid, int size) {
    // Try to satisfy the request before fragmenting the heap with compaction.
    int startIndex = -1;
    if (findAndAllocateFirstFit(pid, size, false, &startIndex)) {
        allocationSizes[pid] = size;
        return true;
    }

    std::cout << "[Memoria] First-Fit fallo. Intentando compactacion...\n";

    compactMemory();

    // Try #2
    startIndex = -1;
    if (findAndAllocateFirstFit(pid, size, true, &startIndex)) {
        allocationSizes[pid] = size;
        return true;
    }

    // If Try 2 fails
    std::cout << "[Memoria] ERROR: No hay suficiente memoria para PID = " 
              << pid << " (" << size << " bloques necesarios)\n";
    return false;
}

void MemoryManager::freeMemory(int pid) {

    // Print the current map to help visualize deallocation effects.
    std::cout << "\n[Memoria] Preparando para liberar memoria del PID=" << pid << "\n";
    printMemory();
    for (int i = 0; i < totalSize; i++) {
        if (memory[i] == pid)
            memory[i] = -1;
    }
    allocationSizes.erase(pid);
    std::cout << "[Memoria] Se ha liberado la memoria ocupada por el PID=" << pid << "\n";
}

void MemoryManager::compactMemory() {
    std::cout << "\n[Memoria] INICIANDO COMPACTACION...\n";
    std::cout << ">>> Memoria ANTES de compactar (Fragmentada) <<<\n";
    printMemory();

    // Move every allocated block to the leftmost positions.
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

int MemoryManager::findBaseAddress(int pid) const {
    for (int i = 0; i < totalSize; i++) {
        if (memory[i] == pid) {
            return i;
        }
    }
    return -1;
}

void MemoryManager::printPageTables() const {
    std::cout << "\n=== Tabla de Paginacion (Simulada) ===\n";
    if (allocationSizes.empty()) {
        std::cout << "(sin procesos en memoria)\n";
        return;
    }

    // Build a stable view sorted by PID for deterministic output.
    std::vector<std::pair<int, int>> entries(allocationSizes.begin(), allocationSizes.end());
    std::sort(entries.begin(), entries.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.first < rhs.first;
    });

    for (const auto& entry : entries) {
        int pid = entry.first;
        int pages = entry.second;
        int base = findBaseAddress(pid);
        if (base == -1) {
            std::cout << "PID " << pid << ": no residente\n";
            continue;
        }

        std::cout << "PID " << pid << " -> ";
        for (int page = 0; page < pages; ++page) {
            int frame = base + page;
            std::cout << "P" << page << "->F" << frame;
            if (page < pages - 1) {
                std::cout << " | ";
            }
        }
        std::cout << "\n";
    }
}

int MemoryManager::translateLogicalToPhysical(int pid, int logicalAddress) const {
    // Reject invalid logical offsets or non-resident processes.
    if (logicalAddress < 0) {
        return -1;
    }

    auto it = allocationSizes.find(pid);
    if (it == allocationSizes.end()) {
        return -1;
    }

    int size = it->second;
    if (logicalAddress >= size) {
        return -1;
    }

    int base = findBaseAddress(pid);
    if (base == -1) {
        return -1;
    }

    return base + logicalAddress;
}
