/*
Módulo encargado del gestor de procesos.
*/

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

class ProcessManager {
public:
    ProcessManager();

    int createProcess(int burstTime, int memoryRequired, int arrivalTime);

    void admitProcessesByTime(int currentTime);

    void runRoundRobin(int q); 

    void printProcessTable() const;
    bool hasReadyProcesses() const;

private:
    int nextPid;
    std::vector<PCB> allprocesses;     // accesible por pid-1
    std::queue<int> readyQueue;

    PCB* getPCB(int pid);
    void transitionToReady(int pid);
    void transitionToRunning(int pid);
    void transitionToTerminated(int pid);
};

#endif
