#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <vector>
#include <iostream>
#include <unordered_map>

// Coordinates contiguous memory using a compacting first-fit allocator.
class MemoryManager {
public:
    // Builds the backing store with all frames marked as free.
    MemoryManager(int totalSize);

    // Attempts to find a first-fit block (optionally after compaction) and reports the start index.
    bool findAndAllocateFirstFit(int pid, int size, bool isPostCompaction, int* allocatedStart = nullptr);
    // Public entry point to allocate memory; performs compaction on demand.
    bool allocateMemoryFirstFit(int pid, int size);
    // Releases every frame owned by the PID and updates bookkeeping.
    void freeMemory(int pid);
    // Dumps the contiguous memory map for debugging.
    void printMemory() const;
    // Packs allocated frames to the left to mitigate external fragmentation.
    void compactMemory();
    // Prints a synthetic page table view for each resident process.
    void printPageTables() const;
    // Translates a logical offset for a PID into a physical frame index.
    int translateLogicalToPhysical(int pid, int logicalAddress) const;

private:
    std::vector<int> memory;
    int totalSize;
    std::unordered_map<int, int> allocationSizes;

    int findBaseAddress(int pid) const;
};

#endif