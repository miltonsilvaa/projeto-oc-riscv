#pragma once

#include <cstdint>
#include <vector>

class Barramento;

class Cache {
public:
    struct LinhaCache {
        bool valida;         
        bool suja;            
        uint32_t tag;        
        uint32_t dados[4];    
        uint32_t contadorLRU;  
    };

    Cache(Barramento& barramento);

    bool ler(uint32_t endereco, uint32_t& dado);

    void escrever(uint32_t endereco, uint32_t dado);

    int getAcertos() const { return acertos; }
    int getErros() const { return erros; }
    double getTaxaAcerto() const;

private:
    std::vector<LinhaCache> linhas;  
    int acertos;                    
    int erros;                      
    Barramento& barramento; 
    uint32_t calcularIndice(uint32_t endereco) const;
    
    uint32_t calcularTag(uint32_t endereco) const;
    
    void preencherLinha(uint32_t indice, uint32_t tag, uint32_t enderecoBase);
    
    uint32_t lerDaMemoria(uint32_t endereco);
    void escreverNaMemoria(uint32_t endereco, uint32_t dado);
};