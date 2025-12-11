#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include "StateTransitions.h"
#include <vector>
#include <queue>

enum class AllocationMode {
    CONTIGOUS,
    PAGED
};

struct PCB {
    int pid;
    StateTransition state;
    int burstTime;      
    int remainingTime; 
    int memoryRequired;
    int arrivalTime; 
    AllocationMode allocationMode;

    PCB(int id, int burst, int mem, int arrival, AllocationMode mode = AllocationMode::CONTIGOUS)
        : pid(id), state(StateTransition::NUEVO),
          burstTime(burst), remainingTime(burst),
          memoryRequired(mem), arrivalTime(arrival), allocationMode(mode) {}
};

class MemoryManager; // Forward declaration

class ProcessManager {
public:
    ProcessManager(MemoryManager* mem);

    int createProcess(int burstTime, int memoryRequired, int arrivalTime,  AllocationMode mode = AllocationMode::CONTIGOUS);

    void admitProcess(int pid);

    void admitProcessesByTime(int currentTime);

    void runRoundRobin(int q); 

    void printProcessTable() const;
    bool hasReadyProcesses() const;

private:
    int nextPid;
    MemoryManager* memoryManager;
    std::vector<PCB> allprocesses;
    std::queue<int> readyQueue;

    PCB* getPCB(int pid);
    void transitionToReady(int pid);
    void transitionToRunning(int pid);
    void transitionToTerminated(int pid);
};

#endif
