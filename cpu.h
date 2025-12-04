#pragma once

#include <cstdint>
#include "barramento.h"
#include "cache.h"  

#include <cstdint>

class Barramento;

class CPU {
public:
    struct EstagioIF_ID {
        uint32_t pc;
        uint32_t instrucao;
        bool valido;
    };
    uint32_t getPC() const { return pc; }
    struct EstagioID_EX {
        uint32_t pc;
        uint32_t instrucao;
        uint32_t opcode;
        uint32_t rd;
        uint32_t rs1;
        uint32_t rs2;
        uint32_t funct3;
        uint32_t funct7;
        int32_t imm;
        uint32_t dadoRs1;
        uint32_t dadoRs2;
        bool valido;
    };

    struct EstagioEX_MEM {
        uint32_t pc;
        uint32_t instrucao;
        uint32_t opcode;
        uint32_t rd;
        uint32_t funct3;  
        uint32_t resultado;
        uint32_t dadoRs2;
        uint32_t enderecoMem;
        bool valido;
        bool escreverMem;
        bool lerMem;
    };

    struct EstagioMEM_WB {
        uint32_t pc;
        uint32_t instrucao;
        uint32_t opcode;
        uint32_t rd;
        uint32_t resultado;
        bool valido;
        bool escreverReg;
    };

    CPU(Cache& cache, Barramento& barramento);
    void executarCiclo(); 

    void habilitarInterrupcoes() { interrupcoesHabilitadas = true; }
    void desabilitarInterrupcoes() { interrupcoesHabilitadas = false; }
    void tratarInterrupcaoExterna(uint8_t codigoInterrupcao);
    void solicitarInterrupcao();

private:
    uint32_t getOpcode(uint32_t instrucao);
    uint32_t getRd(uint32_t instrucao);
    uint32_t getRs1(uint32_t instrucao);
    uint32_t getRs2(uint32_t instrucao);
    uint32_t getFunct3(uint32_t instrucao);
    uint32_t getFunct7(uint32_t instrucao);
    uint32_t regs[32];
    uint32_t pc;
    Cache& cache;
    Barramento& barramento;
    
    EstagioIF_ID if_id;
    EstagioID_EX id_ex;
    EstagioEX_MEM ex_mem;
    EstagioMEM_WB mem_wb;

    bool stall; 
    bool flush;  

    bool interrupcoesHabilitadas;
    bool interrupcao_pendente;
    uint32_t pcSalvo;

    void estagioIF();  
    void estagioID();  
    void estagioEX(); 
    void estagioMEM(); 
    void estagioWB();  

    uint32_t buscarInstrucao(uint32_t endereco);
    void escreverRegistrador(uint32_t indice, uint32_t valor);
    uint32_t lerRegistrador(uint32_t indice);

    int32_t getImmTipoI(uint32_t instrucao);
    int32_t getImmTipoS(uint32_t instrucao);
    int32_t getImmTipoB(uint32_t instrucao);
    int32_t getImmTipoU(uint32_t instrucao);
    int32_t getImmTipoJ(uint32_t instrucao);

    uint32_t executarALU(uint32_t opcode, uint32_t funct3, uint32_t funct7, 
                        uint32_t dado1, uint32_t dado2, int32_t imm);
    
    bool detectarHazardDados();
    void tratarInterrupcao();
    void implementarForwarding();
};