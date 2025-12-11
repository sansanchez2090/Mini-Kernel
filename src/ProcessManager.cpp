#include "ProcessManager.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

ProcessManager::ProcessManager() : nextPid(1) {}

int ProcessManager::createProcess(int burstTime, int memoryRequired) {
    int pid = nextPid++;
    allprocesses.emplace_back(pid, burstTime, memoryRequired);
    std::cout << "[Kernel] Proceso creado...\n PID = " << pid
              << " burst = " << burstTime
              << " mem = " << memoryRequired << "KB\n";
    return pid;
}

PCB* ProcessManager::getPCB(int pid) {
    int index = pid - 1;
    if (index < 0 || index >= (int)allprocesses.size()) return nullptr;
    return &allprocesses[index];
}

void ProcessManager::admitProcess(int pid) {
    PCB* process = getPCB(pid);
    if (!process) return;
    transitionToReady(pid);
    readyQueue.push(pid);
    std::cout << "[Kernel] PID = " << pid << " agregado a la cola de listos\n";
}

bool ProcessManager::hasReadyProcesses() const {
    return !readyQueue.empty();
}

void ProcessManager::transitionToReady(int pid) {
    PCB* process = getPCB(pid);
    if (!process) return;
    process->state = StateTransition::LISTO;
    std::cout << "[Estado] PID = " << pid << " -> LISTO\n";
}

void ProcessManager::transitionToRunning(int pid) {
    PCB* process = getPCB(pid);
    if (!process) return;
    process->state = StateTransition::EJECUCION;
    std::cout << "[Estado] PID = " << pid << " -> EJECUCION\n";
}

void ProcessManager::transitionToTerminated(int pid) {
    PCB* process = getPCB(pid);
    if (!process) return;
    process->state = StateTransition::TERMINADO;
    std::cout << "[Estado] PID = " << pid << " -> TERMINADO\n";
}


void ProcessManager::runRoundRobin(int quantum) {
    std::cout << "[Planificador] Ejecutando Round-Robin (quantum=" << quantum << ")\n";
    while (hasReadyProcesses()) {
        int pid = readyQueue.front();
        readyQueue.pop();

        PCB* process = getPCB(pid);
        if (!process) continue;

        transitionToRunning(pid);

        int runUnits = std::min(quantum, process->remainingTime);
        for (int t = 0; t < runUnits; ++t) {
            std::cout << "[Proceso " << pid << "] ejecutando unidad: " << (process->burstTime - process->remainingTime + t + 1)
                      << "/" << process->burstTime << "\n";
            std::this_thread::sleep_for(300ms);
        }
        process->remainingTime -= runUnits;

        if (process->remainingTime <= 0) {
            transitionToTerminated(pid);
            std::cout << "[Planificador] PID = " << pid << " Terminado\n";
        } else {
            transitionToReady(pid);
            readyQueue.push(pid);
            std::cout << "[Planificador] PID = " << pid << " interrumpido, restan = " << process->remainingTime << "\n";
        }
    }
    std::cout << "[Planificador] Round-Robin completado\n";
}

void ProcessManager::printProcessTable() const {
    std::cout << "PID\tSTATE\t\tBURST\tREMAINING\tMEM\n";
    for (const auto& p : allprocesses) {
        std::cout << p.pid << "\t" << stateTransitionToString(p.state)
                  << "\t\t" << p.burstTime << "\t" << p.remainingTime
                  << "\t\t" << p.memoryRequired << "KB\n";
    }
}
