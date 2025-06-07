#ifndef EVENTO_CHEGADA_HPP
#define EVENTO_CHEGADA_HPP

#include "Evento.hpp"
#include "entidades/Pacote.hpp"
#include <memory>

namespace LogisticSystem {
    class Armazem;
    class Escalonador;
    
    class EventoChegada : public Evento {
    private:
        std::shared_ptr<Pacote> pacote;
        ID_t armazemDestino;
        
        // Referências para processamento
        std::weak_ptr<Armazem> armazem;
        std::weak_ptr<Escalonador> escalonador;
        
    public:
        EventoChegada(std::shared_ptr<Pacote> pct, ID_t destino, Timestamp_t tempo);
        
        // Configuração
        void definirArmazem(std::shared_ptr<Armazem> arm) { armazem = arm; }
        void definirEscalonador(std::shared_ptr<Escalonador> esc) { escalonador = esc; }
        
        // Getters
        std::shared_ptr<Pacote> obterPacote() const { return pacote; }
        ID_t obterArmazemDestino() const { return armazemDestino; }
        
        // Implementações virtuais
        void executar() override;
        std::unique_ptr<Evento> clonar() const override;
        std::string obterDetalhes() const override;
        
    private:
        void processarChegadaFinal();
        void processarChegadaIntermediaria();
        void agendarProximoMovimento();
    };
}

#endif
