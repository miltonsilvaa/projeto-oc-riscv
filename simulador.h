#pragma once

#include "cpu.h"
#include "barramento.h"
#include "memoria.h"
#include <string>
#include <vector>
#include "cache.h" 
#include "teclado.h"
#include "vram.h"

class Simulador {
public:

    Simulador();

    void carregarPrograma(const std::vector<uint8_t>& programa, uint32_t enderecoInicio);

    void executar();

private:

    void atualizarDisplayVRAM();

    VRAM vram;
    Memoria memoria;
    Barramento barramento;
    Cache cache;
    CPU cpu;         
    Teclado teclado;   

    bool emExecucao;

    int contadorCiclosIO;

    static const int CICLOS_PARA_ATUALIZAR_IO = 10000;
};