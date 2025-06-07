#ifndef ESCALONADOR_HPP
#define ESCALONADOR_HPP

#include "utils/Tipos.hpp"
#include "estruturas/FilaPrioridade.hpp"
#include "eventos/Evento.hpp"
#include "interfaces/IProcessadorEvento.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

namespace LogisticSystem {
    struct EstatisticasEscalonador {
        int eventosProcessados;
        int eventosChegada;
        int eventosTransporte;
        Timestamp_t tempoSimulacaoTotal;
        Distance_t tempoMedioProcessamento;
        int eventosDescartados;
        
        EstatisticasEscalonador() : eventosProcessados(0), eventosChegada(0),
                                  eventosTransporte(0), tempoSimulacaoTotal(0.0),
                                  tempoMedioProcessamento(0.0), eventosDescartados(0) {}
    };
    
    class Escalonador {
    private:
        FilaPrioridade<std::shared_ptr<Evento>, Evento::ComparadorTimestamp> filaEventos;
        Timestamp_t relogioSimulacao;
        EstatisticasEscalonador estatisticas;
        
        // Processadores de eventos registrados
        std::unordered_map<TipoEvento, std::vector<std::shared_ptr<IProcessadorEvento>>> processadores;
        
        // Controle de execução
        bool simulacaoAtiva;
        bool modoDebug;
        
        void atualizarEstatisticas(TipoEvento tipo, Timestamp_t tempoProcessamento);
        void logEvento(const std::shared_ptr<Evento>& evento, const std::string& acao) const;
        
    public:
        Escalonador();
        
        // Gerenciamento de eventos
        void agendarEvento(std::shared_ptr<Evento> evento);
        std::shared_ptr<Evento> obterProximoEvento();
        void removerEvento(std::shared_ptr<Evento> evento);
        
        // Controle do relógio
        void avancarRelogio(Timestamp_t novoTempo);
        Timestamp_t obterTempoAtual() const { return relogioSimulacao; }
        
        // Controle da simulação
        bool temEventosPendentes() const { return !filaEventos.vazia(); }
        void pararSimulacao() { simulacaoAtiva = false; }
        void reiniciarSimulacao();
        bool estaAtivo() const { return simulacaoAtiva; }
        
        // Processadores de eventos
        void registrarProcessador(TipoEvento tipo, std::shared_ptr<IProcessadorEvento> processador);
        void removerProcessador(TipoEvento tipo, std::shared_ptr<IProcessadorEvento> processador);
        
        // Execução
        std::shared_ptr<Evento> processarProximoEvento();
        void executarAteTimestamp(Timestamp_t limite);
        void executarNumeroEventos(int quantidade);
        
        // Consultas e estatísticas
        const EstatisticasEscalonador& obterEstatisticas() const { return estatisticas; }
        size_t obterNumeroEventosPendentes() const { return filaEventos.tamanho(); }
        
        std::vector<std::shared_ptr<Evento>> obterEventosPorTipo(TipoEvento tipo) const;
        std::vector<std::shared_ptr<Evento>> obterEventosAteTimestamp(Timestamp_t limite) const;
        
        // Configuração
        void habilitarModoDebug(bool ativo) { modoDebug = ativo; }
        void definirCapacidadeInicial(size_t capacidade);
        
        // Limpeza
        void limpar();
    };
}

#endif
