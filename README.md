# Simulador de Computador RISC-V (RV32I)

> **Projeto de Organização de Computadores**
> Universidade Católica de Santos - Ciência da Computação

Este repositório contém a implementação completa de um simulador de computador baseado na arquitetura **RISC-V (RV32I)**. O projeto simula o ciclo de vida de instruções, gerenciamento de memória, barramento e periféricos de entrada e saída.

## 🚀 Funcionalidades

O sistema foi desenvolvido em C++ e inclui as seguintes características:

| Componente | Status | Detalhes Técnicos |
| :--- | :---: | :--- |
| **CPU Pipeline** | ✅ | Implementação de **5 estágios** (IF, ID, EX, MEM, WB) com detecção de *Hazards* e unidade de *Forwarding*. |
| **Memória Cache** | ✅ | Cache L1 com mapeamento direto e política *Write-Through*. Monitora taxas de *Hit/Miss*. |
| **Interrupções** | ✅ | Sistema de interrupção externa (via teclado simulado) e exceções de ambiente (`ECALL`). |
| **VRAM (Vídeo)** | ✅ | Mapeamento de memória de vídeo (`0x80000`) com renderização periódica no console. |
| **Barramento** | ✅ | Controle centralizado de tráfego de dados e endereçamento entre CPU, Memória e I/O. |

---

## 🏗️ Arquitetura do Sistema

### 1. Mapa de Memória
O simulador respeita rigorosamente o mapeamento solicitado:

| Faixa (Hex) | Dispositivo | Descrição |
| :--- | :--- | :--- |
| `0x00000` - `0x7FFFF` | **RAM Principal** | Armazena o programa (código) e dados. |
| `0x80000` - `0x8FFFF` | **VRAM** | Memória de vídeo. Escritas aqui aparecem no display. |
| `0x9FC00` - `0x9FFFF` | **Periféricos** | Teclado e outros dispositivos de E/S. |

### Fluxo do Pipeline (CPU)
A CPU processa instruções em paralelo através dos estágios:
1.  **IF (Instruction Fetch):** Busca instrução na Cache/Memória.
2.  **ID (Decode):** Decodifica opcode e lê registradores.
3.  **EX (Execute):** Executa operações na ULA (ALU).
4.  **MEM (Memory):** Acessa memória de dados (Load/Store).
5.  **WB (Write Back):** Grava resultados nos registradores.

### I/O Programada e Interrupções
* **Saída:** O sistema verifica periodicamente a VRAM e desenha o conteúdo no terminal.
* **Entrada:** Um módulo de teclado simula a pressão de teclas, gerando sinais de interrupção que pausam o fluxo normal da CPU para tratar o evento.

---

## 🛠️ Tecnologias Utilizadas

* **Linguagem:** C++ (Padrão C++11 ou superior).
* **Compilação:** GCC (g++) ou Clang.
* **Dependências:** Nenhuma biblioteca externa complexa (apenas bibliotecas padrão e APIs de sistema para leitura de teclado).

---

## ⚙️ Como Compilar e Executar

### Compilação
Comando para compilar:

```bash
g++ main.cpp simulador.cpp cpu.cpp memoria.cpp barramento.cpp cache.cpp teclado.cpp -o simulador
```

### Execução
Comando para visualizar ícones:

```bash
chcp 65001
```

Comando para executar:

```bash
.\simulador.exe
```

## 🧪 Exemplo de Saída
Trecho do log de execução demonstrando o funcionamento do Pipeline e da Cache:

```bash
⚡ PIPELINE: Executando ciclo completo
3️⃣ EX: Executando instrucao
   🧮 ALU:  0 op 0 = 7b
🚨 ESTAGIO ID: Decodificando instrucao 0x73
   🔍 OPCODE: 0x73
   🔍 REGISTRADORES: rd=x0, rs1=x0, rs2=x0
   🛑 ECALL DETECTADO - finalizando execução

[Sistema] Parada: ECALL - Programa finalizado

================ VRAM DISPLAY ================
| 🔌 BARRAMENTO: Lendo word @ 0x80000 -> 0x0
. 🔌 BARRAMENTO: Lendo word @ 0x80004 -> 0x0
. 🔌 BARRAMENTO: Lendo word @ 0x80008 -> 0x0
. 🔌 BARRAMENTO: Lendo word @ 0x8000c -> 0x0
. 🔌 BARRAMENTO: Lendo word @ 0x80010 -> 0x0
. 🔌 BARRAMENTO: Lendo word @ 0x80014 -> 0x0
. 🔌 BARRAMENTO: Lendo word @ 0x80018 -> 0x0
. 🔌 BARRAMENTO: Lendo word @ 0x8001c -> 0x0
.  |
| (Tela vazia)                               |
==============================================

>>> EXECUCAO FINALIZADA (3 ciclos) <<<

✅ [main] SIMULADOR FUNCIONOU!
```

###Link do vídeo da apresentação:

[Projeto OC — Demonstração no YouTube](https://youtu.be/reFgKsWUOvA)

### 👥 Autores

* Arthur Batista Correa Alves
* Milton Silva De Jesus
