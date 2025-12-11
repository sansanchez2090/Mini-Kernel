#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <vector>
#include <iostream>
#include <unordered_map>

struct PageTable {
    std::vector<int> frames;
};

class MemoryManager {
public:
    MemoryManager(int totalSize, int frameSize = 4);

    //CONTIGOUS
    bool findAndAllocateFirstFit(int pid, int size, bool isPostCompaction);
    bool allocateMemoryFirstFit(int pid, int size);
    void freeMemory(int pid);
    void printMemory() const;
    void compactMemory();

    //PAGINATION
    int pagesNeededForSize(int size) const;
    bool allocatePagedMemory(int pid, int pagesNeeded);
    void freePagedMemory(int pid);
    void printPageTables() const;
    void printFrameTable() const;
    int getFrameSize() const;

private:
    //CONTIGOUS
    std::vector<int> memory;
    int totalSize;

    //PAGINATION
    int frameSize;         
    int totalFrames;
    std::vector<int> frameTable;
    std::unordered_map<int, PageTable> pageTables;
};

#endif