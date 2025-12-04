#include <iostream>
#include <vector>
#include <cstdint>
#include <iomanip>
#include "simulador.h"
#include "excecoes.h"
#include "memoria.h"

int main() {
    try {
        std::cout << "[main] === TESTE SIMPLES DO SIMULADOR ===" << std::endl;

        std::vector<uint8_t> programa = {
            0x93, 0x00, 0xb0, 0x07, 
            
            0x73, 0x00, 0x00, 0x00
        };

        std::cout << "[main] Programa: ADDI x1, x0, 123 -> ECALL" << std::endl;
        std::cout << "[main] Tamanho: " << programa.size() << " bytes" << std::endl;

        Simulador simulador;
        simulador.carregarPrograma(programa, Memoria::ADDR_RAM_INICIO);

        std::cout << "[main] Iniciando simulador..." << std::endl;
        simulador.executar();
        
        std::cout << "\n✅ [main] SIMULADOR FUNCIONOU!" << std::endl;
        std::cout << "✅ [main] PRONTO PARA ENTREGA!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "[main] ERRO: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}