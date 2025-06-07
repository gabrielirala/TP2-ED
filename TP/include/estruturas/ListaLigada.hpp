#ifndef LISTA_LIGADA_HPP
#define LISTA_LIGADA_HPP

#include <memory>
#include <functional>
#include <stdexcept>

namespace LogisticSystem {
    template<typename T>
    class ListaLigada {
    private:
        struct No {
            T dados;
            std::unique_ptr<No> proximo;
            
            No(const T& valor) : dados(valor), proximo(nullptr) {}
            No(T&& valor) : dados(std::move(valor)), proximo(nullptr) {}
        };
        
        std::unique_ptr<No> primeiro;
        No* ultimo;
        size_t tamanho;
        
    public:
        class Iterator {
        private:
            No* atual;
            
        public:
            Iterator(No* no) : atual(no) {}
            
            T& operator*() { return atual->dados; }
            const T& operator*() const { return atual->dados; }
            T* operator->() { return &atual->dados; }
            const T* operator->() const { return &atual->dados; }
            
            Iterator& operator++() {
                if (atual) atual = atual->proximo.get();
                return *this;
            }
            
            Iterator operator++(int) {
                Iterator temp = *this;
                ++(*this);
                return temp;
            }
            
            bool operator==(const Iterator& other) const {
                return atual == other.atual;
            }
            
            bool operator!=(const Iterator& other) const {
                return !(*this == other);
            }
        };
        
        ListaLigada() : primeiro(nullptr), ultimo(nullptr), tamanho(0) {}
        
        ~ListaLigada() = default;
        
        // Operações básicas
        void inserirInicio(const T& valor);
        void inserirInicio(T&& valor);
        void inserirFim(const T& valor);
        void inserirFim(T&& valor);
        
        T removerInicio();
        T removerFim();
        bool remover(const T& valor);
        bool remover(std::function<bool(const T&)> predicado);
        
        // Consultas
        bool vazia() const { return tamanho == 0; }
        size_t obterTamanho() const { return tamanho; }
        
        const T& obterPrimeiro() const;
        const T& obterUltimo() const;
        T& obterPrimeiro();
        T& obterUltimo();
        
        // Iteradores
        Iterator begin() { return Iterator(primeiro.get()); }
        Iterator end() { return Iterator(nullptr); }
        Iterator begin() const { return Iterator(primeiro.get()); }
        Iterator end() const { return Iterator(nullptr); }
        
        // Operações funcionais
        void paraCada(std::function<void(T&)> funcao);
        void paraCada(std::function<void(const T&)> funcao) const;
        
        template<typename Predicate>
        ListaLigada<T> filtrar(Predicate pred) const;
        
        template<typename Func>
        auto mapear(Func func) -> ListaLigada<decltype(func(std::declval<T>()))>;
        
        void limpar();
    };
}

#endif
