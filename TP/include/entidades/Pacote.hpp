// Esta é uma representação do que Pacote.hpp DEVE ter,
// especialmente a assinatura de atualizarEstado.

#ifndef PACOTE_HPP
#define PACOTE_HPP

#include <string>
#include <vector>
#include "utils/Tipos.hpp" // Inclui os enums e structs

namespace LogisticSystem {

    class Pacote {
    private:
        ID_t idUnico;
        ID_t armazemOrigemInicial;
        ID_t armazemDestinoFinal;
        Timestamp_t dataPostagem;

        EstadoPacote estadoAtual;
        Timestamp_t tempoUltimaAtualizacao;
        std::vector<HistoricoEstado> historico; // Histórico de estados do pacote

        // Rota calculada para o pacote
        // A sua ListaLigada de IDs de armazéns
        ListaLigada<ID_t> rotaArmazens;
        size_t posicaoAtualRota; // Índice atual na rota

        // Estatísticas internas do pacote
        Timestamp_t tempoTotalArmazenado;
        Timestamp_t tempoTotalTransito;
        // ... outras estatísticas que você pode ter

    public:
        // Construtor do pacote ajustado para o novo formato de entrada
        Pacote(ID_t id, Timestamp_t dataPostagem, ID_t origem, ID_t destino);

        // Métodos para atualizar o estado do pacote e registrar no histórico
        // Adiciona um parâmetro para observações detalhadas
        void atualizarEstado(EstadoPacote novoEstado, Timestamp_t timestamp, ID_t armazemId, const std::string& observacoes = "");

        // Métodos de acesso
        ID_t obterIdUnico() const;
        ID_t obterArmazemOrigem() const;
        ID_t obterArmazemDestino() const; // Destino final
        Timestamp_t obterDataPostagem() const;
        EstadoPacote obterEstadoAtual() const;
        Timestamp_t obterTempoUltimaAtualizacao() const;
        const std::string& obterObservacoesUltimaAtualizacao() const;
        const std::vector<HistoricoEstado>& obterHistorico() const;

        // Métodos relacionados à rota
        void definirRota(const ListaLigada<ID_t>& rota);
        ID_t obterProximoArmazem() const; // Próximo armazém na rota
        bool avancarNaRota(); // Move para o próximo armazém na rota, retorna true se não é o último
        bool chegouDestino() const; // Verifica se o pacote chegou ao destino final

        // Métodos de cálculo de métricas (se ainda relevantes para relatórios internos)
        void calcularMetricas(); // Pode recalcular tempo armazenado e transito, etc.

    private:
        // Função auxiliar para atualizar estatísticas internas baseada na transição de estado
        void atualizarEstatisticasInternas(Timestamp_t timestamp);
    };

} // namespace LogisticSystem

#endif // PACOTE_HPP
