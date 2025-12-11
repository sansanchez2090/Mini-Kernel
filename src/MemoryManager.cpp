#include "MemoryManager.h"

MemoryManager::MemoryManager(int totalSize, int frameSize) 
    : totalSize(totalSize), memory(totalSize, -1),
      frameSize(frameSize)
{
    totalFrames = totalSize / frameSize;
    frameTable = std::vector<int>(totalFrames, -1);
    std::cout << "[Memoria] Inicializada con " << totalSize 
              << " Bloques de memoria " << totalFrames 
              << " Frames de tamaño " << frameSize << " bloques.\n";
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

int MemoryManager::pagesNeededForSize(int size) const {
    // tamaño en bloques / frameSize (ceil)
    return (size + frameSize - 1) / frameSize;
}

bool MemoryManager::allocatePagedMemory(int pid, int pagesNeeded) {
    std::vector<int> assigned;

    for (int f = 0; f < totalFrames; ++f) {
        if (frameTable[f] == -1) {
            assigned.push_back(f);
            if ((int)assigned.size() == pagesNeeded) break;
        }
    }

    if ((int)assigned.size() < pagesNeeded) {
        std::cout << "[Paginacion] ERROR: No hay frames libres suficientes para PID = " 
                  << pid << " (necesita " << pagesNeeded << ")\n";
        return false;
    }

    // Reservar frames
    for (int f : assigned) frameTable[f] = pid;

    // Mapear la page table
    pageTables[pid] = PageTable{ assigned };

    std::cout << "[Paginacion] PID = " << pid << " asignado " << pagesNeeded 
              << " paginas (frames:";
    for (int f : assigned) std::cout << " " << f;
    std::cout << " )\n";

    return true;
}

void MemoryManager::freePagedMemory(int pid) {
    if (pageTables.find(pid) == pageTables.end()) return;

    for (int f : pageTables[pid].frames) {
        if (f >= 0 && f < totalFrames) frameTable[f] = -1;
    }
    pageTables.erase(pid);
    std::cout << "[Paginacion] Liberadas paginas de PID=" << pid << "\n";
}

void MemoryManager::printPageTables() const {
    std::cout << "\n=== Page Tables ===\n";
    for (const auto& entry : pageTables) {
        std::cout << "PID " << entry.first << ":";
        for (int f : entry.second.frames) std::cout << " [F" << f << "]";
        std::cout << "\n";
    }
}

void MemoryManager::printFrameTable() const {
    std::cout << "\n=== Frame Table (" << totalFrames << " frames) ===\n";
    for (int i = 0; i < totalFrames; ++i) {
        if (frameTable[i] == -1) std::cout << "[ ]";
        else std::cout << "[" << frameTable[i] << "]";
    }
    std::cout << "\n";
}

int MemoryManager::getFrameSize() const {
    return frameSize;
}

