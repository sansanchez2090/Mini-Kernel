#include "ProcessManager.h"
#include "../include/MemoryManager.h"
#include <iostream>

ProcessManager::ProcessManager(MemoryManager* mem, IOManager* io, FileManager* files)
    : nextPid(1), memoryManager(mem), ioManager(io), fileManager(files) {}

int ProcessManager::createProcess(int burstTime, int memoryRequired, int arrivalTime) {
    // Assign a PID and queue the PCB for later admission.
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
    // Lazily allocate memory once the arrival time is satisfied.
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
    // Iterate every PCB to see who is ready to enter the system.
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

void ProcessManager::scheduleIOAction(int pid, int cpuTrigger, DeviceType device, const std::string& description) {
    // Record when the process should request IO relative to CPU time.
    PCB* process = getPCB(pid);
    if (!process) return;
    process->ioPlan.push_back({cpuTrigger, device, description, false});
}

void ProcessManager::scheduleFileAction(int pid, int cpuTrigger, FileActionType type,
                                        const std::string& name, int sizeBytes) {
    // Queue file operations so they can be replayed during execution.
    PCB* process = getPCB(pid);
    if (!process) return;
    process->filePlan.push_back({cpuTrigger, type, name, sizeBytes, false});
}

void ProcessManager::transitionToReady(int pid) {
    // Update the PCB state and log it for traceability.
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
    // Clean up open descriptors and memory before marking as finished.
    PCB* process = getPCB(pid);
    if (!process) return;
    process->state = StateTransition::TERMINADO;

    if (fileManager) {
        for (const auto& entry : process->openDescriptors) {
            fileManager->closeFile(entry.second);
        }
        process->openDescriptors.clear();
    }

    memoryManager->freeMemory(pid);
    std::cout << "[Estado] PID = " << pid << " -> TERMINADO\n";
}

void ProcessManager::runFileActions(PCB* process) {
    // Execute any pending file script once the CPU time threshold is met.
    if (!fileManager || !process) return;

    for (auto& action : process->filePlan) {
        if (action.executed) continue;
        if (process->cpuTimeExecuted < action.cpuTrigger) continue;

        switch (action.type) {
            case FileActionType::CREATE:
                fileManager->createFile(action.name, action.sizeBytes, process->pid);
                break;
            case FileActionType::OPEN: {
                int descriptor = fileManager->openFile(action.name);
                if (descriptor != -1) {
                    process->openDescriptors[action.name] = descriptor;
                }
                break;
            }
            case FileActionType::CLOSE: {
                auto it = process->openDescriptors.find(action.name);
                if (it != process->openDescriptors.end()) {
                    fileManager->closeFile(it->second);
                    process->openDescriptors.erase(it);
                }
                break;
            }
        }

        action.executed = true;
    }
}

bool ProcessManager::shouldBlockForIO(PCB* process) {
    // Walk the IO script to decide if this quantum should trigger a device.
    if (!ioManager || !process) return false;

    for (auto& script : process->ioPlan) {
        if (script.issued) continue;
        if (process->cpuTimeExecuted < script.cpuTrigger) continue;

        int requestId = ioManager->requestIO(process->pid, script.device, script.description);
        if (requestId == -1) {
            std::cerr << "[IO] Error al solicitar E/S para PID=" << process->pid << "\n";
            return false;
        }

        script.issued = true;
        process->waitingIO = true;
        process->state = StateTransition::BLOQUEADO;
        std::cout << "[Proceso " << process->pid << "] Bloqueado por E/S (req "
                  << requestId << ")\n";
        return true;
    }

    return false;
}

void ProcessManager::handleIOCompletion(const IOEvent& event) {
    // When IO completes, re-queue the process into ready state.
    PCB* process = getPCB(event.pid);
    if (!process) return;
    if (process->state == StateTransition::TERMINADO) return;

    process->waitingIO = false;
    transitionToReady(process->pid);
    readyQueue.push(process->pid);

    std::cout << "[Interrupcion IO] PID=" << event.pid
              << " completo " << event.description << "\n";
}

void ProcessManager::tickIO() {
    if (!ioManager) return;
    ioManager->tick();
    auto completed = ioManager->fetchCompletedEvents();
    for (const auto& event : completed) {
        handleIOCompletion(event);
    }
}


void ProcessManager::runRoundRobin(int quantum) {
    // Main scheduling loop that advances system time and enforces the quantum.
    std::cout << "\n[Planificador] Ejecutando Round Robin (quantum = "
              << quantum << ")...\n";

    int systemTime = 0;

    while (true) {
        std::cout << "\n=== Tiempo " << systemTime << " ===\n";

        admitProcessesByTime(systemTime);

        if (hasReadyProcesses()) {
            // Pop the next PID and give it the CPU.
            int pid = readyQueue.front();
            readyQueue.pop();

            PCB* process = getPCB(pid);
            transitionToRunning(pid);

            runFileActions(process);

            int executedUnits = 0;
            bool blockedThisSlice = false;

            while (executedUnits < quantum && process->remainingTime > 0) {
                // Log each CPU tick to visualize how time slices are consumed.
                std::cout << "[Proceso " << pid << "] Ejecutando en tiempo "
                          << systemTime << " (restantes = "
                          << process->remainingTime << ")\n";

                process->remainingTime--;
                executedUnits++;
                process->cpuTimeExecuted++;
                systemTime++;

                runFileActions(process);

                if (shouldBlockForIO(process)) {
                    // Remember the block so the scheduler does not requeue it.
                    blockedThisSlice = true;
                }

                admitProcessesByTime(systemTime);
                tickIO();

                if (blockedThisSlice || process->remainingTime <= 0) {
                    break;
                }
            }

            if (process->remainingTime <= 0) {
                transitionToTerminated(pid);
                std::cout << "[Planificador] PID = " << pid << " terminado\n";
            }
            else if (blockedThisSlice) {
                std::cout << "[Planificador] PID = " << pid
                          << " bloqueado esperando E/S\n";
            }
            else {
                transitionToReady(pid);
                readyQueue.push(pid);
                std::cout << "[Planificador] PID = " << pid
                          << " interrumpido, restantes="
                          << process->remainingTime << "\n";
            }
        } else {
            // No ready processes means the CPU stays idle for this tick.
            std::cout << "[CPU] Inactivo (Esperando procesos en listos)\n";
            systemTime++;
            admitProcessesByTime(systemTime);
            tickIO();
        }

        bool allTerminated = true;
        for (auto& process : allprocesses) {
            if (process.state != StateTransition::TERMINADO) {
                allTerminated = false;
                break;
            }
        }
        if (allTerminated) {
            break;
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
