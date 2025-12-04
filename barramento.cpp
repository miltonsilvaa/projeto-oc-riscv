#include "barramento.h"
#include <stdexcept>
#include <iostream>  
#include <ios>     
#include <ostream>   

Barramento::Barramento(Memoria& memoria) 
    : memoria(memoria) {
}

static bool isEnderecoDeMemoria(uint32_t endereco) {

    bool emRAM = (endereco >= Memoria::ADDR_RAM_INICIO && 
                  endereco <= Memoria::ADDR_RAM_FIM);
    
    bool emVRAM = (endereco >= Memoria::ADDR_VRAM_INICIO && 
                   endereco <= Memoria::ADDR_VRAM_FIM);
    
    bool emPerifericos = (endereco >= Memoria::ADDR_PERIFERICOS_INICIO && 
                          endereco <= Memoria::ADDR_PERIFERICOS_FIM);

    return emRAM || emVRAM || emPerifericos;
}

uint32_t Barramento::lerWord(uint32_t endereco) {
    std::cout << "🔌 BARRAMENTO: Lendo word @ 0x" << std::hex << endereco;
    
    if (isEnderecoDeMemoria(endereco)) {
        uint32_t valor = memoria.lerWord(endereco);
        std::cout << " -> 0x" << std::hex << valor << std::dec << std::endl;
        return valor;
    }
    
    std::cout << " -> ERRO: Endereço inválido!" << std::endl;
    throw std::out_of_range("Barramento (leitura Word): Endereco fora do mapa: 0x" + std::to_string(endereco));
}

void Barramento::escreverWord(uint32_t endereco, uint32_t dado) {
    std::cout << "🔌 BARRAMENTO: Escrevendo word @ 0x" << std::hex << endereco 
              << " = 0x" << dado;
    
    if (isEnderecoDeMemoria(endereco)) {
        memoria.escreverWord(endereco, dado);
        std::cout << " -> OK" << std::dec << std::endl;
        return;
    }
    
    std::cout << " -> ERRO: Endereço inválido!" << std::dec << std::endl;
    throw std::out_of_range("Barramento (escrita Word): Endereco fora do mapa: 0x" + std::to_string(endereco));
}
uint16_t Barramento::lerHalfWord(uint32_t endereco) {
    // Passo 1: Verifica endereço
    if (isEnderecoDeMemoria(endereco)) {
        // Passo 2: Repassa para memória
        return memoria.lerHalfWord(endereco);
    }
    // Passo 3: Endereço inválido
    throw std::out_of_range("Barramento (leitura HalfWord): Endereco fora do mapa: 0x" + std::to_string(endereco));
}

void Barramento::escreverHalfWord(uint32_t endereco, uint16_t dado) {
    if (isEnderecoDeMemoria(endereco)) {
        memoria.escreverHalfWord(endereco, dado);
        return;
    }
    throw std::out_of_range("Barramento (escrita HalfWord): Endereco fora do mapa: 0x" + std::to_string(endereco));
}

uint8_t Barramento::lerByte(uint32_t endereco) {
    if (isEnderecoDeMemoria(endereco)) {
        return memoria.lerByte(endereco);
    }
    throw std::out_of_range("Barramento (leitura Byte): Endereco fora do mapa: 0x" + std::to_string(endereco));
}

void Barramento::escreverByte(uint32_t endereco, uint8_t dado) {
    if (isEnderecoDeMemoria(endereco)) {
        memoria.escreverByte(endereco, dado);
        return;
    }
    throw std::out_of_range("Barramento (escrita Byte): Endereco fora do mapa: 0x" + std::to_string(endereco));
}