#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include "StateTransitions.h"
#include <vector>
#include <queue>

struct PCB {
    int pid;
    StateTransition state;
    int burstTime;      
    int remainingTime; 
    int memoryRequired;
    int arrivalTime; 

    PCB(int id, int burst, int mem, int arrival)
        : pid(id), state(StateTransition::NUEVO),
          burstTime(burst), remainingTime(burst),
          memoryRequired(mem), arrivalTime(arrival) {}
};

class MemoryManager; // Forward declaration

class ProcessManager {
public:
    ProcessManager(MemoryManager* mem);

    int createProcess(int burstTime, int memoryRequired, int arrivalTime);

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
