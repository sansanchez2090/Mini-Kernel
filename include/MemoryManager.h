#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <vector>
#include <iostream>

class MemoryManager {
public:
    MemoryManager(int totalSize);

    bool allocateMemoryFirstFit(int pid, int size);
    void freeMemory(int pid);
    void printMemory() const;

private:
    std::vector<int> memory;
    int totalSize;
};

#endif