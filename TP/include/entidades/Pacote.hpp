#ifndef PACOTE_HPP
#define PACOTE_HPP

#include "utils/Tipos.hpp"
#include "interfaces/IObservador.hpp"
#include <string>
#include <vector>
#include <memory>

namespace LogisticSystem {
    struct HistoricoEstado {
        EstadoPacote estado;
        Timestamp_t timestamp;
        ID_t armazemId;
        std::string observacoes;
        
        HistoricoEstado(EstadoPacote e, Timestamp_t t, ID_t id, const std::string& obs = "")
            : estado(e), timestamp(t), armazemId(id), observacoes(obs) {}
    };
    
    class Pacote : public IObservavel<HistoricoEstado> {
    private:
        ID_t idUnico;
        Timestamp_t dataPostagem;
        std::string remetente;
        std::string destinatario;
        std::string tipo;
        ID_t armazemOrigem;
        ID_t armazemDestino;
        
        // Roteamento
        std::vector<ID_t> rota;
        size_t posicaoAtualRota;
        
        // Estado e histórico
        EstadoPacote estadoAtual;
        std::vector<HistoricoEstado> historico;
        
        // Estatísticas
        Timestamp_t tempoTotalArmazenado;
        Timestamp_t tempoTotalTransito;
        Timestamp_t timestampUltimaMudanca;
        Timestamp_t tempoEsperadoTotal;
        
        void atualizarEstatisticasInternas(Timestamp_t novoTimestamp);
        
    public:
        Pacote(ID_t id, Timestamp_t postagem, const std::string& rem, 
               const std::string& dest, const std::string& tipoPacote,
               ID_t origem, ID_t destino);
        
        // Getters
        ID_t obterIdUnico() const { return idUnico; }
        Timestamp_t obterDataPostagem() const { return dataPostagem; }
        const std::string& obterRemetente() const { return remetente; }
        const std::string& obterDestinatario() const { return destinatario; }
        const std::string& obterTipo() const { return tipo; }
        ID_t obterArmazemOrigem() const { return armazemOrigem; }
        ID_t obterArmazemDestino() const { return armazemDestino; }
        
        EstadoPacote obterEstadoAtual() const { return estadoAtual; }
        const std::vector<ID_t>& obterRota() const { return rota; }
        size_t obterPosicaoAtualRota() const { return posicaoAtualRota; }
        
        // Roteamento
        void definirRota(const std::vector<ID_t>& novaRota);
        ID_t obterProximoArmazem() const;
        bool chegouDestino() const;
        void avancarNaRota();
        
        // Gerenciamento de estado
        void atualizarEstado(EstadoPacote novoEstado, Timestamp_t timestamp, 
                           ID_t armazemId, const std::string& observacoes = "");
        
        // Estatísticas
        MetricasPacote calcularMetricas() const;
        Timestamp_t obterTempoTotalArmazenado() const { return tempoTotalArmazenado; }
        Timestamp_t obterTempoTotalTransito() const { return tempoTotalTransito; }
        Timestamp_t obterTempoEsperadoTotal() const { return tempoEsperadoTotal; }
        
        void definirTempoEsperado(Timestamp_t tempo) { tempoEsperadoTotal = tempo; }
        
        // Histórico
        const std::vector<HistoricoEstado>& obterHistorico() const { return historico; }
        Timestamp_t obterTimestampUltimaMudanca() const { return timestampUltimaMudanca; }
        
        // Comparadores para ordenação
        struct ComparadorPorTempoArmazenado {
            bool operator()(const std::shared_ptr<Pacote>& a, 
                          const std::shared_ptr<Pacote>& b) const {
                return a->obterTimestampUltimaMudanca() < b->obterTimestampUltimaMudanca();
            }
        };
        
        struct ComparadorPorPrioridade {
            bool operator()(const std::shared_ptr<Pacote>& a, 
                          const std::shared_ptr<Pacote>& b) const {
                // Prioridade: tempo de armazenamento > urgência do tipo
                if (a->obterTempoTotalArmazenado() != b->obterTempoTotalArmazenado()) {
                    return a->obterTempoTotalArmazenado() > b->obterTempoTotalArmazenado();
                }
                return a->obterTipo() < b->obterTipo(); // Critério de desempate
            }
        };
    };
}

#endif
