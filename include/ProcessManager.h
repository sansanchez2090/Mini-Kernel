#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include "StateTransitions.h"
#include "IOManager.h"
#include "FileManager.h"
#include <vector>
#include <queue>
#include <string>
#include <unordered_map>

enum class FileActionType {
    CREATE,
    OPEN,
    CLOSE
};

// Describes when an IO action must be triggered relative to CPU time.
struct ScheduledIO {
    int cpuTrigger;
    DeviceType device;
    std::string description;
    bool issued;
};

// Captures deferred file actions (create/open/close) for a process.
struct ScheduledFileAction {
    int cpuTrigger;
    FileActionType type;
    std::string name;
    int sizeBytes;
    bool executed;
};

// Represents the Process Control Block used by the scheduler.
struct PCB {
    int pid;
    StateTransition state;
    int burstTime;      
    int remainingTime; 
    int memoryRequired;
    int arrivalTime; 
    int cpuTimeExecuted;
    bool waitingIO;
    std::vector<ScheduledIO> ioPlan;
    std::vector<ScheduledFileAction> filePlan;
    std::unordered_map<std::string, int> openDescriptors;

    PCB(int id, int burst, int mem, int arrival)
        : pid(id), state(StateTransition::NUEVO),
          burstTime(burst), remainingTime(burst),
          memoryRequired(mem), arrivalTime(arrival),
          cpuTimeExecuted(0), waitingIO(false) {}
};

// Coordinates memory, IO, and file interactions for every process.
class MemoryManager; // Forward declaration

class ProcessManager {
public:
    ProcessManager(MemoryManager* mem, IOManager* io, FileManager* files);

    // Spawns a PCB with burst/memory/arrival metadata.
    int createProcess(int burstTime, int memoryRequired, int arrivalTime);

    // Moves a process into ready state once memory is available.
    void admitProcess(int pid);

    void admitProcessesByTime(int currentTime);

    // Executes Round-Robin scheduling while orchestrating IO/File actions.
    void runRoundRobin(int q); 

    // Emits the table of processes with their current state.
    void printProcessTable() const;
    bool hasReadyProcesses() const;

    void scheduleIOAction(int pid, int cpuTrigger, DeviceType device, const std::string& description);
    void scheduleFileAction(int pid, int cpuTrigger, FileActionType type, const std::string& name, int sizeBytes = 0);

private:
    int nextPid;
    MemoryManager* memoryManager;
    IOManager* ioManager;
    FileManager* fileManager;
    std::vector<PCB> allprocesses;
    std::queue<int> readyQueue;

    PCB* getPCB(int pid);
    void transitionToReady(int pid);
    void transitionToRunning(int pid);
    void transitionToTerminated(int pid);
    void runFileActions(PCB* process);
    bool shouldBlockForIO(PCB* process);
    void tickIO();
    void handleIOCompletion(const IOEvent& event);
};

#endif
