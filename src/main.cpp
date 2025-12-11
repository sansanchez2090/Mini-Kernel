#include "../include/ProcessManager.h"
#include <iostream>

int main() {
    ProcessManager pm;

    pm.createProcess(6, 100, 0); // P1 llega en t=0
    pm.createProcess(4, 50, 2);  // P2 llega en t=2
    pm.createProcess(7, 70, 5);  // P3 llega en t=5

    pm.runRoundRobin(2);   // Round Robin con quantum 2

    pm.printProcessTable();
    return 0;
}
