/*
Enum to represent state transitions for processes.
*/

#ifndef STATETRANSITIONS_h
#define STATETRANSITIONS_h

#include <string>

enum class StateTransition {
    NUEVO,
    LISTO,
    EJECUCION,
    TERMINADO
};

inline std::string stateTransitionToString(StateTransition state) {
    switch (state) {
        case StateTransition::NUEVO:
            return "NUEVO";
        case StateTransition::LISTO:
            return "LISTO";
        case StateTransition::EJECUCION:
            return "EJECUCION";
        case StateTransition::TERMINADO:
            return "TERMINADO";
        default:
            return "ESTADO_DESCONOCIDO";
    }
}

#endif 