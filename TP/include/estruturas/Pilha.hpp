#ifndef PILHA_HPP
#define PILHA_HPP

#include "ListaLigada.hpp"
#include <stdexcept>

namespace LogisticSystem {
    template<typename T>
    class Pilha {
    private:
        ListaLigada<T> elementos;
        size_t capacidadeMaxima;
        
    public:
        explicit Pilha(size_t capacidade = SIZE_MAX) 
            : capacidadeMaxima(capacidade) {}
        
        // Operações LIFO
        void push(const T& elemento) {
            if (elementos.obterTamanho() >= capacidadeMaxima) {
                throw std::overflow_error("Pilha cheia");
            }
            elementos.inserirInicio(elemento);
        }
        
        void push(T&& elemento) {
            if (elementos.obterTamanho() >= capacidadeMaxima) {
                throw std::overflow_error("Pilha cheia");
            }
            elementos.inserirInicio(std::move(elemento));
        }
        
        T pop() {
            if (elementos.vazia()) {
                throw std::underflow_error("Pilha vazia");
            }
            return elementos.removerInicio();
        }
        
        const T& topo() const {
            if (elementos.vazia()) {
                throw std::underflow_error("Pilha vazia");
            }
            return elementos.obterPrimeiro();
        }
        
        T& topo() {
            if (elementos.vazia()) {
                throw std::underflow_error("Pilha vazia");
            }
            return elementos.obterPrimeiro();
        }
        
        // Operações de consulta
        bool vazia() const { return elementos.vazia(); }
        bool cheia() const { return elementos.obterTamanho() >= capacidadeMaxima; }
        size_t tamanho() const { return elementos.obterTamanho(); }
        size_t capacidade() const { return capacidadeMaxima; }
        
        // Acesso especial para LIFO com profundidade
        const T& obterElemento(size_t posicao) const;
        Distance_t calcularTempoAcesso(size_t posicao, Distance_t tempoUnitario) const;
        
        // Operações de manipulação complexa
        std::vector<T> removerAteElemento(size_t posicao);
        void recolocarElementos(const std::vector<T>& elementos);
        
        void limpar() { elementos.limpar(); }
        
        // Iterator para percorrer do topo para base
        auto begin() { return elementos.begin(); }
        auto end() { return elementos.end(); }
        auto begin() const { return elementos.begin(); }
        auto end() const { return elementos.end(); }
    };
}

#endif
