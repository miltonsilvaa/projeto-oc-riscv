#pragma once

#include <cstdint>
#include <vector>
#include "vram.h" 

class Memoria {
public:
    Memoria(VRAM& vramRef);

    static const uint32_t ADDR_RAM_INICIO = 0x00000;
    static const uint32_t ADDR_RAM_FIM = 0x7FFFF;
    
    static const uint32_t ADDR_VRAM_INICIO = 0x80000;
    static const uint32_t ADDR_VRAM_FIM = 0x8FFFF;
    
    static const uint32_t ADDR_PERIFERICOS_INICIO = 0x9FC00;
    static const uint32_t ADDR_PERIFERICOS_FIM = 0x9FFFF;

    static const uint32_t TAMANHO_TOTAL_MEMORIA = ADDR_PERIFERICOS_FIM + 1;

    uint32_t lerWord(uint32_t endereco);

    void escreverWord(uint32_t endereco, uint32_t dado);

    uint16_t lerHalfWord(uint32_t endereco);

    void escreverHalfWord(uint32_t endereco, uint16_t dado);

    uint8_t lerByte(uint32_t endereco);

    void escreverByte(uint32_t endereco, uint8_t dado);

    void carregarPrograma(const std::vector<uint8_t>& programa, uint32_t inicio);

    uint8_t* getPonteiroVRAM();

private:
    VRAM& vram; 

    std::vector<uint8_t> dados;

    void verificarEndereco(uint32_t endereco);
};