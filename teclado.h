#pragma once

#include <cstdint>

class Teclado {
public:
    Teclado();
    
    bool verificarInterrupcao();
    
    uint8_t lerTecla();
    
    void simularTecla(uint8_t tecla);

private:
    uint8_t teclaPressionada;
    bool interrupcaoPendente;
};