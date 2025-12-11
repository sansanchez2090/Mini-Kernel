#include "../include/ProcessManager.h"
#include "../include/MemoryManager.h"
#include "../include/IOManager.h"
#include "../include/FileManager.h"

#include <iostream>

int main() {
    // Bootstraps memory, IO, and file managers to run an integrated RR scenario.
    MemoryManager mem(30);
    FileManager fileManager;
    IOManager ioManager;
    ProcessManager pm(&mem, &ioManager, &fileManager);

    std::cout << "\n======================================================================\n";
    std::cout << "--- ESCENARIO INTEGRADO (RR, Quantum=2) ---\n";
    std::cout << " Procesos + Memoria + Archivos + E/S \n";
    std::cout << "======================================================================\n";

    int p1 = pm.createProcess(12, 10, 0);
    int p2 = pm.createProcess(10, 8, 2);
    int p3 = pm.createProcess(8, 6, 4);
    int p4 = pm.createProcess(7, 12, 6);

    pm.scheduleFileAction(p1, 0, FileActionType::CREATE, "p1.log", 512);
    pm.scheduleFileAction(p1, 1, FileActionType::OPEN, "p1.log");
    pm.scheduleIOAction(p1, 3, DeviceType::DISCO, "Lectura de disco P1");
    pm.scheduleFileAction(p1, 6, FileActionType::CLOSE, "p1.log");

    pm.scheduleFileAction(p2, 0, FileActionType::CREATE, "tabla_proc.txt", 256);
    pm.scheduleIOAction(p2, 2, DeviceType::TECLADO, "Lectura teclado P2");
    pm.scheduleFileAction(p2, 3, FileActionType::OPEN, "tabla_proc.txt");

    pm.scheduleIOAction(p3, 1, DeviceType::RED, "Envio paquete P3");
    pm.scheduleFileAction(p3, 2, FileActionType::OPEN, "tabla_proc.txt");
    pm.scheduleFileAction(p3, 4, FileActionType::CLOSE, "tabla_proc.txt");

    pm.scheduleIOAction(p4, 0, DeviceType::DISCO, "Carga inicial P4");

    pm.runRoundRobin(2);

    std::cout << "\n=== RESULTADOS FINALES DE LA SIMULACION ===\n";
    mem.printMemory();
    mem.printPageTables();
    pm.printProcessTable();
    fileManager.printDirectory();
    fileManager.printOpenFiles();
    ioManager.printDeviceTable();
    ioManager.printPendingRequests();

    return 0;
}