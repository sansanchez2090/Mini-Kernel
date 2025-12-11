#include "ProcessManager.h"
#include "MemoryManager.h"

int main() {

    MemoryManager mem(40, 4); // 40 bloques, frameSize=4 -> 10 frames
    ProcessManager pm(&mem);

    pm.createProcess(6, 6, 0, AllocationMode::CONTIGOUS); // necesita 6 bloques
    pm.createProcess(4, 10, 1, AllocationMode::PAGED);     // necesita 10 bloques -> 3 pages (4 per page)
    pm.createProcess(5, 8, 2, AllocationMode::PAGED);

    pm.runRoundRobin(2);

    mem.printMemory();
    mem.printFrameTable();
    mem.printPageTables();

    pm.printProcessTable();
}

    //1. Inicialización de la simulación MEMORIA CONTIGUA
    // MemoryManager mem(30);   // Memoria de 30 bloques
    // ProcessManager pm(&mem);
    
    // std::cout << "\n======================================================================\n";
    // std::cout << "--- ESCENARIO DE COMPACTACION (Round-Robin, Quantum=2) ---\n";
    // std::cout << " Objetivo: Fragmentar memoria y obligar a P4 a compactar para asignarse. \n";
    // std::cout << "======================================================================\n";

    // // 2. Creación de Procesos que Fragmentarán la Memoria (Llegan a T=0)
    
    // // P1 (Grande): R=12, M=10KB. Dejará un hueco grande al final.
    // pm.createProcess(12, 10, 0); 
    // // P2 (Grande): R=12, M=10KB. Dejará un hueco grande al final.
    // pm.createProcess(12, 10, 0); 
    // // P3 (Pequeño): R=5, M=5KB. Terminará pronto, dejando un hueco pequeño.
    // pm.createProcess(5, 5, 0); 

    // // P1, P2 y P3 ocupan [0-24]. Restan 5 bloques libres.

    // // 3. Proceso que FALLARÁ Inicialmente y Forzará la Compactación
    
    // // P4: Necesita 15KB. No cabrá si P1, P2 y P3 han terminado en orden RR.
    // // Lo hacemos llegar tarde para que la fragmentación ocurra durante la ejecución.
    // pm.createProcess(10, 15, 15); 
    
    // // 4. Ejecución de la Simulación
    // pm.runRoundRobin(2); // Ejecución con Quantum = 2

    // // 5. Resultados Finales
    // std::cout << "\n=== RESULTADOS FINALES DE LA SIMULACION ===\n";
    // mem.printMemory();
    // pm.printProcessTable();