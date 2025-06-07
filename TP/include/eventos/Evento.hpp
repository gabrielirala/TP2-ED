#ifndef EVENTO_HPP
#define EVENTO_HPP

#include "utils/Tipos.hpp"
#include <memory>
#include <string>

namespace LogisticSystem {
    class Evento {
    protected:
        TipoEvento tipo;
        Timestamp_t timestamp;
        int prioridade;
        std::string descricao;
        
    public:
        Evento(TipoEvento tipoEvento, Timestamp_t tempo, int prio = 0, 
               const std::string& desc = "");
        
        virtual ~Evento() = default;
        
        // Getters
        TipoEvento obterTipo() const { return tipo; }
        Timestamp_t obterTimestamp() const { return timestamp; }
        int obterPrioridade() const { return prioridade; }
        const std::string& obterDescricao() const { return descricao; }
        
        // Operações virtuais
        virtual void executar() = 0;
        virtual std::unique_ptr<Evento> clonar() const = 0;
        virtual std::string obterDetalhes() const = 0;
        
        // Comparadores para fila de prioridade
        struct ComparadorTimestamp {
            bool operator()(const std::shared_ptr<Evento>& a, 
                          const std::shared_ptr<Evento>& b) const {
                if (a->obterTimestamp() != b->obterTimestamp()) {
                    return a->obterTimestamp() > b->obterTimestamp(); // Min-heap
                }
                return a->obterPrioridade() > b->obterPrioridade();
            }
        };
        
        // Métodos utilitários
        bool operator<(const Evento& other) const;
        bool operator>(const Evento& other) const;
        bool operator==(const Evento& other) const;
    };
}

#endif
