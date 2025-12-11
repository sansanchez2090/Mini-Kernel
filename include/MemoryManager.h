#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <vector>
#include <iostream>

class MemoryManager {
public:
    MemoryManager(int totalSize);

    bool findAndAllocateFirstFit(int pid, int size, bool isPostCompaction);
    bool allocateMemoryFirstFit(int pid, int size);
    void freeMemory(int pid);
    void printMemory() const;
    void compactMemory();

private:
    std::vector<int> memory;
    int totalSize;
};

#endif