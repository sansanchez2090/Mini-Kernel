/*
Enum para las transiciones de estado en un sistema de gestión de procesos.
*/

#ifndef STATETRANSITIONS_h
#define STATETRANSITIONS_h

#include <string>

using namespace std;

enum class StateTransition {
    NUEVO,
    LISTO,
    EJECUCION,
    TERMINADO
};

string stateTransitionToString(StateTransition state) {
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