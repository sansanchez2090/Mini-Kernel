#include "../include/ProcessManager.h"
#include "../include/MemoryManager.h"

int main() {

    MemoryManager mem(30);   // memoria de 30 bloques
    ProcessManager pm(&mem);

    pm.createProcess(6, 6, 0);   // burst=6, mem=6 bloques, arrival=0
    pm.createProcess(4, 5, 2);
    pm.createProcess(7, 8, 5);

    pm.runRoundRobin(2);

    mem.printMemory();
    pm.printProcessTable();

    return 0;
}
