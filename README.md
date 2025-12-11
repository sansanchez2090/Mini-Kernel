# Mini-Kernel de Demostración

Simulador académico que integra planificación de procesos, gestión de memoria, sistema de archivos y dispositivos de E/S para prácticas del curso de Sistemas Operativos. El proyecto está escrito en C++17 y reproduce el flujo de un mini kernel por medio de trazas en consola.

## Arquitectura general

El repositorio se organiza en dos directorios principales:

- `include/`: cabeceras de los módulos (`ProcessManager`, `MemoryManager`, `FileManager`, `IOManager`, `StateTransitions`).
- `src/`: implementaciones concretas y `main.cpp` con el escenario integrado.

Cada módulo se mantiene independiente y se enlaza en `main.cpp` para ejecutar la simulación completa.

## Módulos y algoritmos

### Gestión de procesos
- **PCB**: estructura `PCB` con `pid`, `estado`, ráfagas, memoria requerida, tiempo de llegada, tiempo de CPU ejecutado, plan de E/S y plan de archivos.
- **Cola de listos**: `std::queue<int>` gestionada por `ProcessManager`.
- **Planificador**: Round-Robin con quantum configurable (`runRoundRobin`). Avanza unidad por unidad, dispara operaciones de archivo, solicita E/S y procesa interrupciones.
- **Transiciones**: `NUEVO → LISTO → EJECUCION → BLOQUEADO → LISTO → … → TERMINADO`. El estado `BLOQUEADO` aparece durante E/S.

### Gestión de memoria
- **Memoria principal**: vector `memory` con identificadores de PID.
- **Asignación**: algoritmo First-Fit con intento de compactación cuando falla la primera búsqueda (`allocateMemoryFirstFit`).
- **Liberación**: `freeMemory` recorre el vector y marca los bloques como libres.
- **Visualización**: `printMemory` muestra el mapa de bloques ocupado/vacío.
- **Memoria virtual**: tabla simulada por proceso (`printPageTables`) y función `translateLogicalToPhysical` para mapear direcciones lógicas → físicas dentro de la región asignada a cada PID.

### Gestión de archivos
- **Directorios y FCB**: `RootDirectory` mantiene un `unordered_map` con `FileControlBlock` (id, nombre, tamaño, PID dueño, estado abierto y contador de aperturas).
- **Operaciones**: `FileManager` permite `createFile`, `openFile`, `closeFile`, imprime el directorio y la tabla de archivos abiertos.
- **Integración**: el `ProcessManager` planifica eventos de archivos y cierra los descriptores abiertos cuando el proceso termina.

### Gestión de E/S
- **Tabla de dispositivos**: `IOManager` registra teclado, disco y NIC con latencias predeterminadas.
- **Solicitudes**: `requestIO` crea estructuras `IORequest`; si el dispositivo está ocupado se encola, de lo contrario se ejecuta.
- **Simulación de tiempo**: `tick` decrementa la latencia restante de cada solicitud, genera eventos de interrupción (`IOEvent`) y activa la siguiente del mismo dispositivo.
- **Integración**: `ProcessManager` cambia el estado del proceso a `BLOQUEADO`, lo saca de la CPU y vuelve a `LISTO` cuando recibe el evento completado.

## Escenario de simulación

`src/main.cpp` define un escenario con cuatro procesos (`p1`…`p4`) que:
- Solicitan memoria (con fragmentación y compactación si se necesita).
- Ejecutan ráfagas Round-Robin (quantum = 2).
- Realizan operaciones de archivo programadas (crear, abrir, cerrar).
- Generan eventos de E/S contra distintos dispositivos.

La salida final muestra mapas de memoria, tablas de páginas, estado de procesos, estado del directorio y tabla de dispositivos.

## Instrucciones de compilación y ejecución

Requisitos: `g++` con soporte C++17 (MinGW o similar en Windows, gcc/clang en Linux). Desde la raíz del proyecto:

```powershell
g++ src/*.cpp -I include -std=c++17 -o main.exe
./main.exe
```

En Linux/macOS cambia `./main.exe` por `./main`.

## Pruebas sugeridas

- Modificar el `quantum` y observar cómo cambia el orden de finalización.
- Ajustar la memoria total o el tamaño de procesos para forzar fragmentación y verificar la compactación.
- Añadir nuevos eventos de archivo/E/S con `scheduleFileAction` y `scheduleIOAction` para confirmar que los procesos se bloquean y reanudan correctamente.
- Usar `MemoryManager::translateLogicalToPhysical(pid, logical)` dentro del `main` para inspeccionar direcciones específicas.

## Trabajo futuro

- Incorporar más dispositivos o prioridades de E/S.
- Agregar métricas de rendimiento (tiempo de espera, turnaround).
- Crear múltiples escenarios de prueba y automatizarlos con scripts.
- Añadir una capa de interfaz gráfica o web para visualizar los estados en tiempo real.
