/*
Enum to represent state transitions for processes.
*/

#ifndef STATETRANSITIONS_h
#define STATETRANSITIONS_h

#include <string>

// Captures every lifecycle state a process can go through.
enum class StateTransition {
    NUEVO,
    LISTO,
    EJECUCION,
    BLOQUEADO,
    TERMINADO
};

// Converts internal enum values into readable labels for logs and tables.
inline std::string stateTransitionToString(StateTransition state) {
    switch (state) {
        case StateTransition::NUEVO:
            return "NUEVO";
        case StateTransition::LISTO:
            return "LISTO";
        case StateTransition::EJECUCION:
            return "EJECUCION";
        case StateTransition::BLOQUEADO:
            return "BLOQUEADO";
        case StateTransition::TERMINADO:
            return "TERMINADO";
        default:
            return "ESTADO_DESCONOCIDO";
    }
}

#endif