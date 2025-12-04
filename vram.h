#pragma once
#include <cstdint>
#include <iostream>
#include <string>

class VRAM {
private:
    uint8_t vram[0x10000];
    int cicloRender;

public:
    VRAM() : cicloRender(0) {
        for (int i = 0; i < 0x10000; i++) {
            vram[i] = ' ';
        }
        std::cout << "🖥️ VRAM: Inicializada (64KB @ 0x80000-0x8FFFF)" << std::endl;
    }
    
    void escrever(uint32_t endereco, uint32_t valor) {
        if (endereco >= 0x80000 && endereco <= 0x8FFFF) {
            uint32_t offset = endereco - 0x80000;
            vram[offset] = valor & 0xFF;
            std::cout << "🖥️ VRAM: Escrito '" << (char)(valor & 0xFF) 
                      << "' (0x" << std::hex << (valor & 0xFF) 
                      << ") @ 0x" << endereco << std::dec << std::endl;
        }
    }
    
    uint32_t ler(uint32_t endereco) {
        if (endereco >= 0x80000 && endereco <= 0x8FFFF) {
            return vram[endereco - 0x80000];
        }
        return 0;
    }
    
    void renderizar(int ciclosExecutados) {
        cicloRender++;
        if (cicloRender >= 5) {
            std::cout << "\n" << std::string(50, '=') << std::endl;
            std::cout << "🖥️ SAIDA VRAM (Ciclo " << ciclosExecutados << ")" << std::endl;
            std::cout << std::string(50, '=') << std::endl;
            
            bool vramVazia = true;
            for (int y = 0; y < 10; y++) { 
                std::cout << "|";
                for (int x = 0; x < 40; x++) { 
                    char c = vram[y * 80 + x];
                    if (c != 0 && c != ' ') {
                        std::cout << c;
                        vramVazia = false;
                    } else {
                        std::cout << " ";
                    }
                }
                std::cout << "|" << std::endl;
            }
            
            if (vramVazia) {
                std::cout << "| (VRAM vazia - use SW para escrever caracteres) |" << std::endl;
            }
            std::cout << std::string(50, '=') << std::endl;
            cicloRender = 0;
        }
    }
    
    void limpar() {
        for (int i = 0; i < 0x10000; i++) {
            vram[i] = ' ';
        }
        std::cout << "🖥️ VRAM: Tela limpa" << std::endl;
    }
};