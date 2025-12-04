#include "cache.h"
#include "barramento.h"
#include <iostream>

Cache::Cache(Barramento& barramento) 
    : linhas(256), acertos(0), erros(0), barramento(barramento) {
    
    for (auto& linha : linhas) {
        linha.valida = false;
        linha.suja = false;
        linha.tag = 0;
        linha.contadorLRU = 0;
        for (int i = 0; i < 4; i++) {
            linha.dados[i] = 0;
        }
    }
    std::cout << "🧠 CACHE: Cache inicializada com 256 linhas conectada ao barramento" << std::endl;
}

bool Cache::ler(uint32_t endereco, uint32_t& dado) {
    uint32_t indice = calcularIndice(endereco);
    uint32_t tag = calcularTag(endereco);
    uint32_t offset = (endereco >> 2) & 0x3;

    LinhaCache& linha = linhas[indice];

    if (linha.valida && linha.tag == tag) {
        dado = linha.dados[offset];
        linha.contadorLRU++;
        acertos++;
        
        std::cout << "🧠 CACHE: HIT! Endereço 0x" << std::hex << endereco 
                  << " -> Dado: 0x" << dado << std::dec << std::endl;
        return true;
    } else {
        erros++;
        std::cout << "🧠 CACHE: MISS! Endereço 0x" << std::hex << endereco << std::dec << std::endl;
      
        if (linha.valida && linha.suja) {
            std::cout << "🧠 CACHE: Escrevendo linha suja de volta na memória" << std::endl;
            uint32_t enderecoBase = (linha.tag << 8) | indice;
            for (int i = 0; i < 4; i++) {
                escreverNaMemoria(enderecoBase + (i * 4), linha.dados[i]);
            }
        }
        
        uint32_t enderecoBase = endereco & ~0xF; 
        preencherLinha(indice, tag, enderecoBase);
        
        dado = linha.dados[offset];
        return false;
    }
}

void Cache::escrever(uint32_t endereco, uint32_t dado) {
    uint32_t indice = calcularIndice(endereco);
    uint32_t tag = calcularTag(endereco);
    uint32_t offset = (endereco >> 2) & 0x3;

    LinhaCache& linha = linhas[indice];

    if (linha.valida && linha.tag == tag) {
        linha.dados[offset] = dado;
        linha.suja = true; 
        linha.contadorLRU++;
        acertos++;
        std::cout << "🧠 CACHE: HIT escrita! Endereço 0x" << std::hex << endereco 
                  << " = 0x" << dado << std::dec << std::endl;
    } else {
        erros++;
        std::cout << "🧠 CACHE: MISS escrita! Escrita direta na memória" << std::endl;
    }
    
    escreverNaMemoria(endereco, dado);
}

double Cache::getTaxaAcerto() const {
    int total = acertos + erros;
    return total > 0 ? (static_cast<double>(acertos) / total) * 100.0 : 0.0;
}

uint32_t Cache::calcularIndice(uint32_t endereco) const {
    return (endereco >> 4) & 0xFF; 
}

uint32_t Cache::calcularTag(uint32_t endereco) const {
    return endereco >> 12; 
}

void Cache::preencherLinha(uint32_t indice, uint32_t tag, uint32_t enderecoBase) {
    LinhaCache& linha = linhas[indice];
    
    linha.valida = true;
    linha.suja = false;
    linha.tag = tag;
    linha.contadorLRU++;
    
    for (int i = 0; i < 4; i++) {
        linha.dados[i] = lerDaMemoria(enderecoBase + (i * 4));
    }
    
    std::cout << "🧠 CACHE: Linha " << indice << " preenchida com bloco 0x" 
              << std::hex << enderecoBase << std::dec << std::endl;
}

uint32_t Cache::lerDaMemoria(uint32_t endereco) {
    std::cout << "🧠 CACHE: Lendo da memória via barramento @ 0x" << std::hex << endereco << std::dec << std::endl;
    return barramento.lerWord(endereco);
}

void Cache::escreverNaMemoria(uint32_t endereco, uint32_t dado) {
    std::cout << "🧠 CACHE: Escrevendo na memória via barramento @ 0x" << std::hex << endereco 
              << " = 0x" << dado << std::dec << std::endl;
    barramento.escreverWord(endereco, dado);
}