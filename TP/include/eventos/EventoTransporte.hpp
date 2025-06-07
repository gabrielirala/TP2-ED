#ifndef EVENTO_TRANSPORTE_HPP
#define EVENTO_TRANSPORTE_HPP

#include "Evento.hpp"
#include "entidades/SistemaTransporte.hpp"
#include <vector>
#include <memory>

namespace LogisticSystem {
    class Armazem;
    class Escalonador;
    
    class EventoTransporte : public Evento {
    private:
        ID_t armazemOrigem;
        ID_t armazemDestino;
        
        // Referências para processamento
        std::weak_ptr<Armazem> armazemOrigemRef;
        std::weak_ptr<Armazem> armazemDestinoRef;
        std::weak_ptr<SistemaTransporte> sistemaTransporte;
        std::weak_ptr<Escalonador> escalonador;
        
    public:
        EventoTransporte(ID_t origem, ID_t destino, Timestamp_t tempo);
        
        // Configuração
        void definirArmazemOrigem(std::shared_ptr<Armazem> origem) { armazemOrigemRef = origem; }
        void definirArmazemDestino(std::shared_ptr<Armazem> destino) { armazemDestinoRef = destino; }
        void definirSistemaTransporte(std::shared_ptr<SistemaTransporte> sistema) { sistemaTransporte = sistema; }
        void definirEscalonador(std::shared_ptr<Escalonador> esc) { escalonador = esc; }
        
        // Getters
        ID_t obterArmazemOrigem() const { return armazemOrigem; }
        ID_t obterArmazemDestino() const { return armazemDestino; }
        
        // Implementações virtuais
        void executar() override;
        std::unique_ptr<Evento> clonar() const override;
        std::string obterDetalhes() const override;
        
    private:
        void executarTransporte();
        void agendarChegadas(const std::vector<std::shared_ptr<Pacote>>& pacotes);
        void agendarProximoTransporte();
        bool validarPrecondições() const;
    };
}

#endif
