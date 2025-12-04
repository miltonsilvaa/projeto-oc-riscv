#pragma once

#include <cstdint>
#include "memoria.h"

class Barramento {
public:

    Barramento(Memoria& memoria);

    uint32_t lerWord(uint32_t endereco);

    void escreverWord(uint32_t endereco, uint32_t dado);

    uint16_t lerHalfWord(uint32_t endereco);

    void escreverHalfWord(uint32_t endereco, uint16_t dado);

    uint8_t lerByte(uint32_t endereco);

    void escreverByte(uint32_t endereco, uint8_t dado);

private:

    Memoria& memoria;
};