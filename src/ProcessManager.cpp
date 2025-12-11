#include "ProcessManager.h"
#include "../include/MemoryManager.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

ProcessManager::ProcessManager(MemoryManager* mem) : nextPid(1), memoryManager(mem) {}

int ProcessManager::createProcess(int burstTime, int memoryRequired, int arrivalTime) {
    int pid = nextPid++;
    allprocesses.emplace_back(pid, burstTime, memoryRequired, arrivalTime);
    std::cout << "[Kernel] Proceso creado...\n PID = " << pid
              << " Tiempo de llegada = " << arrivalTime
              << " rafaga = " << burstTime
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

    // try to allocate memory
    if (!memoryManager->allocateMemoryFirstFit(pid, process->memoryRequired)) {
        std::cout << "[Planificador] PID=" << pid 
                  << " rechazado (no hay memoria disponible)\n";
        return;
    }

    transitionToReady(pid);
    readyQueue.push(pid);

    std::cout << "[Planificador] PID=" << pid 
              << " agregado a los listos\n";
}

void ProcessManager::admitProcessesByTime(int currentTime) {
    for (auto& process : allprocesses) {
        if (process.state == StateTransition::NUEVO && process.arrivalTime <= currentTime) {
            std::cout << "[Tiempo " << currentTime << "] PID = " << process.pid 
                      << " llego\n";
            admitProcess(process.pid);
        }
    }
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

    memoryManager->freeMemory(pid);
    std::cout << "[Estado] PID = " << pid << " -> TERMINADO\n";
}


void ProcessManager::runRoundRobin(int quantum) {
    std::cout << "\n[Planificador] Ejecutando Round Robin (quantum = " 
              << quantum << ")...\n";

    int systemTime = 0;
    bool allTerminated = false;

    while (!allTerminated) {

        std::cout << "\n=== Tiempo " << systemTime << " ===\n";

        admitProcessesByTime(systemTime);

        if (hasReadyProcesses()) {
            int pid = readyQueue.front();
            readyQueue.pop();

            PCB* process = getPCB(pid);
            transitionToRunning(pid);

            int executedUnits = 0;

            // Advance up to quantum or until process finishes
            while (executedUnits < quantum && process->remainingTime > 0) {

                std::cout << "[Proceso " << pid << "] Ejecutando en tiempo " 
                          << systemTime << " (restantes = " 
                          << process->remainingTime << ")\n";

                // Advance one time unit
                std::this_thread::sleep_for(300ms);
                process->remainingTime--;
                executedUnits++;

                // Global time increment
                systemTime++;

                // Admit new arrivals in the global time
                std::cout << "\n=== Tiempo " << systemTime << " ===\n";
                admitProcessesByTime(systemTime);
            }

            if (process->remainingTime <= 0) {
                transitionToTerminated(pid);
                std::cout << "[Planificador] PID = " << pid << " terminado\n";
            }
            else {
                transitionToReady(pid);
                readyQueue.push(pid);
                std::cout << "[Planificador] PID = " << pid 
                          << " interrumpido, restantes=" 
                          << process->remainingTime << "\n";
            }
        }
        else {
            // No READY processes
            std::cout << "[CPU] Inactivo (Esperando procesos en listos)\n";
            std::this_thread::sleep_for(300ms);
            systemTime++;
        }

        allTerminated = true;
        for (auto& process : allprocesses) {
            if (process.state != StateTransition::TERMINADO) {
                allTerminated = false;
                break;
            }
        }
    }

    std::cout << "\n[Planificador] Round-Robin completado.\n";

}

void ProcessManager::printProcessTable() const {
    std::cout << "PID\tESTADO\t\tRAFAGA\tRESTANTES\tMEM\n";
    for (const auto& p : allprocesses) {
        std::cout << p.pid << "\t" << stateTransitionToString(p.state)
                  << "\t" << p.burstTime << "\t" << p.remainingTime
                  << "\t" << p.memoryRequired << "KB\n";
    }
}
