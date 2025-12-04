#include "simulador.h"
#include <iostream>
#include <iomanip>

Simulador::Simulador() 
    : vram(),             
      memoria(vram),        
      barramento(memoria),  
      cache(barramento),    
      cpu(cache, barramento),
      teclado(),             
      emExecucao(false),
      contadorCiclosIO(0)
{
    std::cout << "[Hardware] Sistema inicializado e conectado." << std::endl;
}

void Simulador::carregarPrograma(const std::vector<uint8_t>& programa, uint32_t enderecoInicio) {
    std::cout << "[Loader] Carregando " << programa.size() << " bytes @ 0x" 
              << std::hex << enderecoInicio << "..." << std::endl;

    for (size_t i = 0; i < programa.size(); ++i) {
        try {
            barramento.escreverByte(enderecoInicio + i, programa[i]);
        } catch (...) {
            std::cerr << "ERRO FATAL: Falha ao escrever na memoria (Byte " << i << ")" << std::endl;
        }
    }

    try {
        uint8_t check = barramento.lerByte(enderecoInicio);
        std::cout << "   [Check] 0x" << std::hex << enderecoInicio << " = 0x" << (int)check << std::endl;
    } catch(...) {}
}

void Simulador::atualizarDisplayVRAM() {
    std::cout << "\n================ VRAM DISPLAY ================" << std::endl;
    std::cout << "| ";
    bool vazio = true;
    
    for (uint32_t addr = 0x80000; addr < 0x80020; addr += 4) {
        try {
            uint32_t val = barramento.lerWord(addr);
            char c = (char)(val & 0xFF);
            
            if (c > 32 && c < 127) { 
                std::cout << c << " ";
                vazio = false;
            } else {
                std::cout << ". ";
            }
        } catch(...) {}
    }
    std::cout << " |" << std::endl;
    
    if (vazio) std::cout << "| (Tela vazia)                               |" << std::endl;
    std::cout << "==============================================" << std::endl;
}

void Simulador::executar() {
    std::cout << "\n>>> INICIANDO EXECUCAO <<<" << std::endl;
    
    emExecucao = true;
    int ciclo = 0;
    int maxCiclos = 100; 

    while (emExecucao && ciclo < maxCiclos) {
        ciclo++;
        std::cout << "\n--- Ciclo " << std::dec << ciclo << " ---" << std::endl;

        if (ciclo == 8) {
            std::cout << "[Interrupcao] Simulando tecla 'A'..." << std::endl;
            cpu.tratarInterrupcaoExterna(1); 
        }

        try {
            cpu.executarCiclo(); 

            if (cpu.getPC() > 0x100) { 
                std::cout << "[Sistema] PC fora do programa - finalizando execução" << std::endl;
                emExecucao = false;
                break;
            }

            if (ciclo % 5 == 0) {
                atualizarDisplayVRAM();
            }

        } catch (const std::exception& e) {
            std::cout << "\n[Sistema] Parada: " << e.what() << std::endl;
            emExecucao = false;
        }
    }
    
    if (ciclo >= maxCiclos) {
        std::cout << "⚠️ AVISO: Limite de " << maxCiclos << " ciclos atingido!" << std::endl;
    }
    
    atualizarDisplayVRAM();
    std::cout << "\n>>> EXECUCAO FINALIZADA (" << ciclo << " ciclos) <<<" << std::endl;
}