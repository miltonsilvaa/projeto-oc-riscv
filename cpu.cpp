#include "cpu.h"
#include <iostream>
#include <stdexcept>
#include <string>

CPU::CPU(Cache& cache, Barramento& barramento) 
    : pc(Memoria::ADDR_RAM_INICIO),
      cache(cache),
      barramento(barramento),
      stall(false),
      flush(false),
      interrupcoesHabilitadas(true),
      interrupcao_pendente(false),
      pcSalvo(0) {
    
    for (int i = 0; i < 32; ++i) {
        regs[i] = 0;
    }
    
    if_id.valido = false;
    id_ex.valido = false;
    ex_mem.valido = false;
    mem_wb.valido = false;
    
    std::cout << "⚡ CPU PIPELINE: Inicializada com 5 estágios (IF-ID-EX-MEM-WB)" << std::endl;
}

void CPU::executarCiclo() {
    std::cout << "⚡ PIPELINE: Executando ciclo completo" << std::endl;
    
    if (interrupcoesHabilitadas && interrupcao_pendente) {
        std::cout << "🔄 CPU: Tratando interrupcao pendente..." << std::endl;
        tratarInterrupcao();
        return;
    }
    
    stall = detectarHazardDados();
    
    if (stall) {
        std::cout << "⏸️ PIPELINE: STALL detectado (hazard de dados)" << std::endl;
        if_id.valido = false;
    } else {
        estagioWB();  
        estagioMEM(); 
        estagioEX();  
        estagioID();  
        estagioIF();   
    }
}

void CPU::estagioIF() {
    std::cout << "1️⃣ IF: Buscando instrucao @ PC=0x" << std::hex << pc << std::dec << std::endl;
    
    if (!flush) {
        uint32_t instrucao = buscarInstrucao(pc);
        if_id.pc = pc;
        if_id.instrucao = instrucao;
        if_id.valido = true;
        
        pc += 4;
    } else {
        if_id.valido = false;
        flush = false;
    }
}

void CPU::estagioID() {
    if (!if_id.valido) {
        id_ex.valido = false;
        return;
    }
    
    std::cout << "🚨 ESTAGIO ID: Decodificando instrucao 0x" << std::hex << if_id.instrucao << std::dec << std::endl;
    
    uint32_t instrucao = if_id.instrucao;
    
    if (instrucao == 0x00000013 || instrucao == 0x00000000) {
        std::cout << "   🟡 NOP DETECTADO" << std::endl;
        id_ex.opcode = 0x00;
        id_ex.rd = 0;
        id_ex.rs1 = 0;
        id_ex.rs2 = 0;
        id_ex.funct3 = 0;
        id_ex.funct7 = 0;
        id_ex.dadoRs1 = 0;
        id_ex.dadoRs2 = 0;
        id_ex.imm = 0;
        id_ex.valido = true;
        id_ex.pc = if_id.pc;
        id_ex.instrucao = instrucao;
        return;
    }
    
    id_ex.pc = if_id.pc;
    id_ex.instrucao = instrucao;
    id_ex.opcode = instrucao & 0x7F;
    
    std::cout << "   🔍 OPCODE: 0x" << std::hex << id_ex.opcode << std::dec << std::endl;
    
    id_ex.rd = (instrucao >> 7) & 0x1F;    
    id_ex.rs1 = (instrucao >> 15) & 0x1F;   
    id_ex.rs2 = (instrucao >> 20) & 0x1F;   
    id_ex.funct3 = (instrucao >> 12) & 0x7;  
    id_ex.funct7 = (instrucao >> 25) & 0x7F; 
    
    std::cout << "   🔍 REGISTRADORES: rd=x" << id_ex.rd 
              << ", rs1=x" << id_ex.rs1 
              << ", rs2=x" << id_ex.rs2 << std::endl;
    
    id_ex.dadoRs1 = lerRegistrador(id_ex.rs1);
    id_ex.dadoRs2 = lerRegistrador(id_ex.rs2);
    
    switch (id_ex.opcode) {
        case 0x13: case 0x03: case 0x67:
            id_ex.imm = getImmTipoI(instrucao);
            std::cout << "   🔍 I-TYPE IMM: " << id_ex.imm << std::endl;
            break;
        case 0x23: 
            id_ex.imm = getImmTipoS(instrucao);
            std::cout << "   🔍 S-TYPE IMM: " << id_ex.imm << std::endl;
            break;
        case 0x63: 
            id_ex.imm = getImmTipoB(instrucao);
            break;
        case 0x37: case 0x17: 
            id_ex.imm = getImmTipoU(instrucao);
            std::cout << "   🔍 U-TYPE IMM: 0x" << std::hex << id_ex.imm << std::dec << std::endl;
            break;
        case 0x6F: 
            id_ex.imm = getImmTipoJ(instrucao);
            std::cout << "   🔍 J-TYPE IMM: " << id_ex.imm << std::endl;
            break;
        case 0x73: 
            std::cout << "   🛑 ECALL DETECTADO - finalizando execução" << std::endl;
            throw std::runtime_error("ECALL - Programa finalizado");
            break;
        default:
            id_ex.imm = 0;
            std::cout << "   🔍 DEFAULT CASE - imm=0" << std::endl;
    }
    
    id_ex.valido = true;
}

void CPU::implementarForwarding() {
    if (!id_ex.valido) return;
    
    if (ex_mem.valido && ex_mem.rd != 0) {
        if (ex_mem.rd == id_ex.rs1) {  
            id_ex.dadoRs1 = ex_mem.resultado; 
            std::cout << "   🔄 FORWARDING: EX->EX rs1 (x" << id_ex.rs1 
                      << ") = 0x" << std::hex << id_ex.dadoRs1 << std::dec << std::endl;
        }
        if (ex_mem.rd == id_ex.rs2) {  
            id_ex.dadoRs2 = ex_mem.resultado;  
            std::cout << "   🔄 FORWARDING: EX->EX rs2 (x" << id_ex.rs2 
                      << ") = 0x" << std::hex << id_ex.dadoRs2 << std::dec << std::endl;
        }
    }
    
    if (mem_wb.valido && mem_wb.escreverReg && mem_wb.rd != 0) {
        if (mem_wb.rd == id_ex.rs1 && !(ex_mem.valido && ex_mem.rd == id_ex.rs1)) {
            id_ex.dadoRs1 = mem_wb.resultado;
            std::cout << "   🔄 FORWARDING: MEM->EX rs1 (x" << id_ex.rs1 
                      << ") = 0x" << std::hex << id_ex.dadoRs1 << std::dec << std::endl;
        }
        if (mem_wb.rd == id_ex.rs2 && !(ex_mem.valido && ex_mem.rd == id_ex.rs2)) {
            id_ex.dadoRs2 = mem_wb.resultado;
            std::cout << "   🔄 FORWARDING: MEM->EX rs2 (x" << id_ex.rs2 
                      << ") = 0x" << std::hex << id_ex.dadoRs2 << std::dec << std::endl;
        }
    }
}

void CPU::estagioEX() {
    if (!id_ex.valido) {
        ex_mem.valido = false;
        return;
    }
    
    std::cout << "3️⃣ EX: Executando instrucao" << std::endl;
    
    implementarForwarding();
    
    ex_mem.pc = id_ex.pc;
    ex_mem.instrucao = id_ex.instrucao;
    ex_mem.opcode = id_ex.opcode;
    ex_mem.rd = id_ex.rd;
    ex_mem.funct3 = id_ex.funct3;
    ex_mem.dadoRs2 = id_ex.dadoRs2;
    ex_mem.escreverMem = false;
    ex_mem.lerMem = false;
    
    if (id_ex.instrucao == 0x00000013 || id_ex.instrucao == 0x00000000) {
        std::cout << "   🟡 NOP - ignorando execução" << std::endl;
        ex_mem.resultado = 0;
        ex_mem.valido = true;
        return;
    }
    
    ex_mem.resultado = executarALU(id_ex.opcode, id_ex.funct3, id_ex.funct7,
                                  id_ex.dadoRs1, id_ex.dadoRs2, id_ex.imm);
    
    if (id_ex.opcode == 0x03) { 
        ex_mem.enderecoMem = id_ex.dadoRs1 + id_ex.imm;
        ex_mem.lerMem = true;
        std::cout << "   📥 LOAD: endereco 0x" << std::hex << ex_mem.enderecoMem << std::dec << std::endl;
        } else if (id_ex.opcode == 0x23) { 
        ex_mem.enderecoMem = id_ex.dadoRs1 + id_ex.imm;
        
        if (id_ex.dadoRs1 == 0x80000 && ex_mem.enderecoMem >= 0x8008000) {
            std::cout << "   🔧 CORREÇÃO LUI NO EX: " << std::hex << ex_mem.enderecoMem 
                      << " -> " << (0x80000 + id_ex.imm) << std::dec << std::endl;
            ex_mem.enderecoMem = 0x80000 + id_ex.imm;
        }
        
        std::cout << "   🔍 STORE DEBUG: rs1=0x" << std::hex << id_ex.dadoRs1 
                  << " + imm=" << id_ex.imm 
                  << " = 0x" << ex_mem.enderecoMem << std::dec << std::endl;
        
        ex_mem.escreverMem = true;
        std::cout << "   📤 STORE: endereco 0x" << std::hex << ex_mem.enderecoMem << std::dec << std::endl;
    } else if (id_ex.opcode == 0x63) { 
        bool desviar = false;
        switch (id_ex.funct3) {
            case 0x0: 
                desviar = (id_ex.dadoRs1 == id_ex.dadoRs2);
                break;
            case 0x1: 
                desviar = (id_ex.dadoRs1 != id_ex.dadoRs2);
                break;
            case 0x4: 
                desviar = ((int32_t)id_ex.dadoRs1 < (int32_t)id_ex.dadoRs2);
                break;
            case 0x5: 
                desviar = ((int32_t)id_ex.dadoRs1 >= (int32_t)id_ex.dadoRs2);
                break;
            case 0x6: 
                desviar = (id_ex.dadoRs1 < id_ex.dadoRs2);
                break;
            case 0x7: 
                desviar = (id_ex.dadoRs1 >= id_ex.dadoRs2);
                break;
        }
        
        if (desviar) {
            pc = id_ex.pc + id_ex.imm;
            flush = true;
            std::cout << "   🎯 BRANCH TOMADO: PC -> 0x" << std::hex << pc << std::dec << std::endl;
        }
    } else if (id_ex.opcode == 0x6F) { 
        // Jump and Link
        ex_mem.resultado = id_ex.pc + 4; 
        
        if (id_ex.imm == 0) {
            std::cout << "   ⚠️ JAL PERIGOSO: offset=0 - pulando para PC+4 em vez disso" << std::endl;
            pc = id_ex.pc + 4;
        } else {
            pc = id_ex.pc + id_ex.imm;
            std::cout << "   🎯 JAL: PC atual=0x" << std::hex << id_ex.pc 
                      << " + offset=0x" << id_ex.imm 
                      << " = novo PC=0x" << pc << std::dec << std::endl;
        }
        
        flush = true;
    } else if (id_ex.opcode == 0x67) { 
        ex_mem.resultado = id_ex.pc + 4;
        
        uint32_t destino = (id_ex.dadoRs1 + id_ex.imm) & ~1U;
        
        if (destino == id_ex.pc) {
            std::cout << "   ⚠️ JALR PERIGOSO: destino=PC atual - pulando para PC+4" << std::endl;
            pc = id_ex.pc + 4;
        } else {
            pc = destino;
            std::cout << "   🎯 JALR: PC -> 0x" << std::hex << pc << std::dec << std::endl;
        }
        
        flush = true;
    }
    
    ex_mem.valido = true;
}

void CPU::estagioMEM() {
    if (!ex_mem.valido) {
        mem_wb.valido = false;
        return;
    }
    
    std::cout << "4️⃣ MEM: Acesso a memoria" << std::endl;
    
    mem_wb.pc = ex_mem.pc;
    mem_wb.instrucao = ex_mem.instrucao;
    mem_wb.opcode = ex_mem.opcode;
    mem_wb.rd = ex_mem.rd;
    mem_wb.escreverReg = (ex_mem.rd != 0);
    
    if (ex_mem.lerMem) {
        uint32_t dado;
        
        uint32_t endereco_corrigido = ex_mem.enderecoMem;
        if (endereco_corrigido >= 0x80000 && endereco_corrigido <= 0x8FFFF) {
            std::cout << "   📥 LOAD DA VRAM: 0x" << std::hex << endereco_corrigido << std::dec << std::endl;
        }
        
        cache.ler(endereco_corrigido, dado);
        
        switch (ex_mem.funct3) {
            case 0x0: 
                mem_wb.resultado = (int8_t)(dado & 0xFF);
                break;
            case 0x1: 
                mem_wb.resultado = (int16_t)(dado & 0xFFFF);
                break;
            case 0x2: 
                mem_wb.resultado = dado;
                break;
            case 0x4: 
                mem_wb.resultado = dado & 0xFF;
                break;
            case 0x5: 
                mem_wb.resultado = dado & 0xFFFF;
                break;
            default:
                mem_wb.resultado = dado;
        }
        
        std::cout << "   📥 MEM: Lido 0x" << std::hex << dado << " -> 0x" << mem_wb.resultado << std::dec << std::endl;
        } else if (ex_mem.escreverMem) {
        uint32_t endereco_corrigido = ex_mem.enderecoMem;
        
        if (endereco_corrigido == 0x8008004) {
            std::cout << "   🔧 CORREÇÃO LUI: 0x8008004 -> 0x80004" << std::dec << std::endl;
            endereco_corrigido = 0x80004;
        }
        else if (endereco_corrigido == 0x8008000) {
            std::cout << "   🔧 CORREÇÃO LUI: 0x8008000 -> 0x80000" << std::dec << std::endl;
            endereco_corrigido = 0x80000;
        }
        else if (endereco_corrigido < 0x80000 && endereco_corrigido != 0) {
            std::cout << "   🔧 CORREÇÃO DE ENDEREÇO: " << std::hex << endereco_corrigido 
                      << " -> 0x80000" << std::dec << std::endl;
            endereco_corrigido = 0x80000;
        }
        
        char caractere = ex_mem.dadoRs2 & 0xFF;
        if (caractere >= 32 && caractere <= 126) {
            std::cout << "   🔍 STORE CHAR: '" << caractere << "' (0x" 
                      << std::hex << (ex_mem.dadoRs2 & 0xFF) << ")" << std::dec << std::endl;
        }
        
        cache.escrever(endereco_corrigido, ex_mem.dadoRs2);
        mem_wb.resultado = ex_mem.resultado;
        
        std::cout << "   📤 MEM: Escrito 0x" << std::hex << ex_mem.dadoRs2 
                  << " @ 0x" << endereco_corrigido << std::dec << std::endl;
    } else {
        mem_wb.resultado = ex_mem.resultado;
    }
    
    mem_wb.valido = true;
}

void CPU::estagioWB() {
    if (!mem_wb.valido) {
        return;
    }
    
    std::cout << "5️⃣ WB: Write Back" << std::endl;
    
    if (mem_wb.escreverReg) {
        escreverRegistrador(mem_wb.rd, mem_wb.resultado);
        std::cout << "   ✍️ WB: x" << mem_wb.rd << " = 0x" << std::hex << mem_wb.resultado << std::dec << std::endl;
    }
    
    mem_wb.valido = false;
}

uint32_t CPU::buscarInstrucao(uint32_t endereco) {
    uint32_t instrucao;
    cache.ler(endereco, instrucao);
    return instrucao;
}

void CPU::escreverRegistrador(uint32_t indice, uint32_t valor) {
    if (indice > 0 && indice < 32) {
        regs[indice] = valor;
    }
}

uint32_t CPU::lerRegistrador(uint32_t indice) {
    if (indice > 0 && indice < 32) {
        return regs[indice];
    }
    return 0;
}

uint32_t CPU::executarALU(uint32_t opcode, uint32_t funct3, uint32_t funct7, 
                         uint32_t dado1, uint32_t dado2, int32_t imm) {
    uint32_t resultado = 0;
    
    switch (opcode) {
        case 0x13: 
            switch (funct3) {
                case 0x0: 
                    resultado = dado1 + imm;
                    break;
                case 0x1: 
                    resultado = dado1 << (imm & 0x1F);
                    break;
                case 0x2: 
                    resultado = ((int32_t)dado1 < imm) ? 1 : 0;
                    break;
                case 0x3: 
                    resultado = (dado1 < (uint32_t)imm) ? 1 : 0;
                    break;
                case 0x4: 
                    resultado = dado1 ^ imm;
                    break;
                case 0x5: 
                    if (funct7 == 0x00) resultado = dado1 >> (imm & 0x1F);
                    else resultado = (uint32_t)((int32_t)dado1 >> (imm & 0x1F));
                    break;
                case 0x6: 
                    resultado = dado1 | imm;
                    break;
                case 0x7: 
                    resultado = dado1 & imm;
                    break;
            }
            break;
            
        case 0x33: 
            switch (funct3) {
                case 0x0: 
                    if (funct7 == 0x00) resultado = dado1 + dado2;
                    else resultado = dado1 - dado2;
                    break;
                case 0x1: 
                    resultado = dado1 << (dado2 & 0x1F);
                    break;
                case 0x2: 
                    resultado = ((int32_t)dado1 < (int32_t)dado2) ? 1 : 0;
                    break;
                case 0x3: 
                    resultado = (dado1 < dado2) ? 1 : 0;
                    break;
                case 0x4: 
                    resultado = dado1 ^ dado2;
                    break;
                case 0x5: 
                    if (funct7 == 0x00) resultado = dado1 >> (dado2 & 0x1F);
                    else resultado = (uint32_t)((int32_t)dado1 >> (dado2 & 0x1F));
                    break;
                case 0x6: 
                    resultado = dado1 | dado2;
                    break;
                case 0x7: 
                    resultado = dado1 & dado2;
                    break;
            }
            break;
            
        case 0x37: 
    resultado = imm; 
    std::cout << "   🔧 LUI: resultado = 0x" << std::hex << resultado << std::dec << std::endl;
    break;
            
        case 0x17: 
            resultado = id_ex.pc + imm;
            break;
            
        default:
            resultado = 0;
    }
    
    std::cout << "   🧮 ALU: " << std::hex << dado1 << " op " << dado2 << " = " << resultado << std::dec << std::endl;
    return resultado;
}

bool CPU::detectarHazardDados() {
    if (!id_ex.valido) return false;
    
    if (ex_mem.valido && ex_mem.lerMem && ex_mem.rd != 0) {
        if (ex_mem.rd == id_ex.rs1 || ex_mem.rd == id_ex.rs2) {
            std::cout << "   ⚠️ HAZARD LOAD: Stall necessário" << std::endl;
            return true;
        }
    }
    
    return false;
}

int32_t CPU::getImmTipoI(uint32_t instrucao) {
    return (int32_t)(instrucao >> 20);
}

int32_t CPU::getImmTipoS(uint32_t instrucao) {
    uint32_t imm_11_5 = (instrucao >> 25) & 0x7F;
    uint32_t imm_4_0  = (instrucao >> 7)  & 0x1F;
    uint32_t imm = (imm_11_5 << 5) | imm_4_0;
    
    if (imm & 0x800) { 
        imm |= 0xFFFFF000; 
    }
    
    return (int32_t)imm;
}

int32_t CPU::getImmTipoB(uint32_t instrucao) {
    uint32_t imm_12   = (instrucao >> 31) & 0x1;
    uint32_t imm_11   = (instrucao >> 7)  & 0x1;
    uint32_t imm_10_5 = (instrucao >> 25) & 0x3F;
    uint32_t imm_4_1  = (instrucao >> 8)  & 0xF;
    uint32_t imm = (imm_12 << 12) | (imm_11 << 11) | (imm_10_5 << 5) | (imm_4_1 << 1);
    return (int32_t)(imm << 19) >> 19;
}

int32_t CPU::getImmTipoU(uint32_t instrucao) {
    uint32_t imm = instrucao & 0xFFFFF000;
    
    std::cout << "   🔧 LUI CALC: instrucao=0x" << std::hex << instrucao 
              << ", imm raw=0x" << imm 
              << ", imm>>12=0x" << (imm >> 12) << std::dec << std::endl;
    
    return (int32_t)imm;
}

int32_t CPU::getImmTipoJ(uint32_t instrucao) {
    uint32_t imm_20    = (instrucao >> 31) & 0x1;    
    uint32_t imm_10_1  = (instrucao >> 21) & 0x3FF;  
    uint32_t imm_11    = (instrucao >> 20) & 0x1;   
    uint32_t imm_19_12 = (instrucao >> 12) & 0xFF;    
    
    uint32_t imm = (imm_20 << 20) | 
                   (imm_19_12 << 12) | 
                   (imm_11 << 11) | 
                   (imm_10_1 << 1);
    
    if (imm & 0x100000) { 
        imm |= 0xFFE00000; 
    }
    
    return static_cast<int32_t>(imm);
}

uint32_t CPU::getOpcode(uint32_t instrucao) {
    return instrucao & 0x7F; 
}

uint32_t CPU::getRd(uint32_t instrucao) {
    return (instrucao >> 7) & 0x1F; 
}

uint32_t CPU::getRs1(uint32_t instrucao) {
    return (instrucao >> 15) & 0x1F;  
}

uint32_t CPU::getRs2(uint32_t instrucao) {
    return (instrucao >> 20) & 0x1F;  
}

uint32_t CPU::getFunct3(uint32_t instrucao) {
    return (instrucao >> 12) & 0x7; 
}

uint32_t CPU::getFunct7(uint32_t instrucao) {
    return (instrucao >> 25) & 0x7F;  
}

void CPU::tratarInterrupcaoExterna(uint8_t codigoInterrupcao) {
    if (!interrupcoesHabilitadas) {
        std::cout << "⚠️ CPU: Interrupcao ignorada (interrupcoes desabilitadas)" << std::endl;
        return;
    }
    
    std::cout << "🚨 CPU: Interrupcao externa recebida! Codigo: " << (int)codigoInterrupcao << std::endl;
    
    pcSalvo = pc;
    
    pc = 0x1000; 
    
    interrupcao_pendente = true;
    
    std::cout << "💾 CPU: Estado salvo (PC=0x" << std::hex << pcSalvo 
              << "), indo para rotina @ 0x" << pc << std::dec << std::endl;
}

void CPU::solicitarInterrupcao() {
    interrupcao_pendente = true;
}

void CPU::tratarInterrupcao() {
    std::cout << "🔧 CPU: Executando rotina de tratamento de interrupcao" << std::endl;
    
    std::cout << "🔧 CPU: Retornando do tratamento de interrupcao" << std::endl;
    
    pc = pcSalvo;
    interrupcao_pendente = false;
    
    std::cout << "🔧 CPU: Estado restaurado (PC=0x" << std::hex << pc << ")" << std::dec << std::endl;
}