#include "memoria.h"
#include <stdexcept>
#include <iostream>

Memoria::Memoria(VRAM& vramRef) : vram(vramRef) {
    dados.resize(TAMANHO_TOTAL_MEMORIA, 0);
    std::cout << "💾 MEMORIA: Inicializada com VRAM integrada" << std::endl;
}

void Memoria::verificarEndereco(uint32_t endereco) {
    if (endereco >= TAMANHO_TOTAL_MEMORIA) {
        throw std::out_of_range("Acesso de memoria fora dos limites: 0x" + std::to_string(endereco));
    }
}

uint8_t Memoria::lerByte(uint32_t endereco) {
    verificarEndereco(endereco);
    
    return dados[endereco];
}

void Memoria::escreverByte(uint32_t endereco, uint8_t dado) {
    verificarEndereco(endereco);
    
    if (endereco >= ADDR_VRAM_INICIO && endereco <= ADDR_VRAM_FIM) {
        vram.escrever(endereco, dado);
        std::cout << "🖥️ MEMORIA: Escrito na VRAM @ 0x" 
                  << std::hex << endereco << " = '" << (char)dado 
                  << "' (0x" << (int)dado << ")" << std::dec << std::endl;
    }
    
    dados[endereco] = dado;
}

uint16_t Memoria::lerHalfWord(uint32_t endereco) {
    uint16_t b0 = lerByte(endereco);
    
    uint16_t b1 = lerByte(endereco + 1);
    
    return b0 | (b1 << 8);
}

void Memoria::escreverHalfWord(uint32_t endereco, uint16_t dado) {
    escreverByte(endereco, (dado & 0xFF));
    
    escreverByte(endereco + 1, (dado >> 8) & 0xFF);
}

uint32_t Memoria::lerWord(uint32_t endereco) {
    uint32_t b0 = lerByte(endereco);    
    uint32_t b1 = lerByte(endereco + 1);  
    uint32_t b2 = lerByte(endereco + 2);   
    uint32_t b3 = lerByte(endereco + 3);  
    
    return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
}

void Memoria::escreverWord(uint32_t endereco, uint32_t dado) {
    escreverByte(endereco, (dado & 0xFF));
     
    escreverByte(endereco + 1, (dado >> 8) & 0xFF);
    
    escreverByte(endereco + 2, (dado >> 16) & 0xFF);
    
    escreverByte(endereco + 3, (dado >> 24) & 0xFF);
}

void Memoria::carregarPrograma(const std::vector<uint8_t>& programa, uint32_t inicio) {
    for (size_t i = 0; i < programa.size(); ++i) {
        if (inicio + i >= TAMANHO_TOTAL_MEMORIA) {
            break; 
        }
        
        dados[inicio + i] = programa[i];
    }
}

uint8_t* Memoria::getPonteiroVRAM() {
    return &dados[ADDR_VRAM_INICIO];
}