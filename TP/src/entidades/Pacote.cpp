#include "entidades/Pacote.hpp"
#include <iostream> // Para debug ou mensagens de erro

namespace LogisticSystem {

    // Construtor do pacote ajustado para o novo formato de entrada (sem remetente/destinatario/tipo)
    Pacote::Pacote(ID_t id, Timestamp_t dataPostagem, ID_t origem, ID_t destino)
        : idUnico(id), armazemOrigemInicial(origem), armazemDestinoFinal(destino),
          dataPostagem(dataPostagem), estadoAtual(EstadoPacote::NAO_POSTADO),
          tempoUltimaAtualizacao(dataPostagem), posicaoAtualRota(0),
          tempoTotalArmazenado(0.0), tempoTotalTransito(0.0) {
        // O primeiro registro no histórico é a postagem (NAO_POSTADO ou CHEGADA_ESCALONADA)
        historico.push_back({dataPostagem, EstadoPacote::NAO_POSTADO, origem, "Pacote criado, aguardando postagem."});
    }

    // Método para atualizar o estado do pacote e registrar no histórico
    void Pacote::atualizarEstado(EstadoPacote novoEstado, Timestamp_t timestamp, ID_t armazemId, const std::string& observacoes) {
        // Calcula o tempo que o pacote passou no estado anterior
        if (estadoAtual == EstadoPacote::ARMAZENADO) {
            tempoTotalArmazenado += (timestamp - tempoUltimaAtualizacao);
        } else if (estadoAtual == EstadoPacote::REMOVIDO_PARA_TRANSPORTE) { // Pacote em trânsito
            tempoTotalTransito += (timestamp - tempoUltimaAtualizacao);
        }
        // Nota: A lógica de tempoTotalTransito deve ser mais robusta,
        // pois pacotes em trânsito não estão em um armazém específico.
        // A duração do transporte é medida entre o agendamento e a chegada.

        estadoAtual = novoEstado;
        tempoUltimaAtualizacao = timestamp;

        // Adiciona o novo estado ao histórico
        historico.push_back({timestamp, novoEstado, armazemId, observacoes});

        // Notificar observadores (se houver, como o Simulador ou GerenciadorEstatisticas)
        // Isso depende da sua implementação de IObservador e do sistema de notificação.
        // Ex: notificadores.notify(shared_from_this());
    }

    // ... (Outros métodos de Pacote: obterIdUnico, obterArmazemOrigem, etc.) ...

    ID_t Pacote::obterIdUnico() const { return idUnico; }
    ID_t Pacote::obterArmazemOrigem() const { return armazemOrigemInicial; }
    ID_t Pacote::obterArmazemDestino() const { return armazemDestinoFinal; }
    Timestamp_t Pacote::obterDataPostagem() const { return dataPostagem; }
    EstadoPacote Pacote::obterEstadoAtual() const { return estadoAtual; }
    Timestamp_t Pacote::obterTempoUltimaAtualizacao() const { return tempoUltimaAtualizacao; }
    const std::string& Pacote::obterObservacoesUltimaAtualizacao() const {
        if (historico.empty()) return ""; // Ou lance uma exceção
        return historico.back().observacoes;
    }
    const std::vector<HistoricoEstado>& Pacote::obterHistorico() const { return historico; }

    void Pacote::definirRota(const ListaLigada<ID_t>& rota) {
        this->rotaArmazens = rota;
        this->posicaoAtualRota = 0;
    }

    ID_t Pacote::obterProximoArmazem() const {
        if (posicaoAtualRota < rotaArmazens.obterTamanho()) {
            return rotaArmazens.obterElemento(posicaoAtualRota);
        }
        return -1; // Sinaliza que não há próximo armazém ou chegou ao destino final
    }

    bool Pacote::avancarNaRota() {
        if (posicaoAtualRota < rotaArmazens.obterTamanho() - 1) {
            posicaoAtualRota++;
            return true;
        }
        return false; // Chegou ao destino final ou rota inválida
    }

    bool Pacote::chegouDestino() const {
        return posicaoAtualRota == rotaArmazens.obterTamanho() - 1;
    }

    // Se você tiver uma função para recalcular métricas baseadas no histórico completo
    void Pacote::calcularMetricas() {
        // Implementar cálculo de tempo de estadia, tempo em trânsito, etc.,
        // iterando sobre o `historico` do pacote.
        // Isso é mais robusto do que a atualização incremental em `atualizarEstado`
        // se houver transições de estado complexas ou dados faltantes.
    }

    // A função `atualizarEstatisticasInternas` pode ser removida ou incorporada em `atualizarEstado`
    // visto que a lógica de cálculo de tempo já está em `atualizarEstado` agora.
    void Pacote::atualizarEstatisticasInternas(Timestamp_t timestamp) {
        // Esta função pode ser incorporada diretamente em atualizarEstado
        // ou usada para lógica mais complexa. Por enquanto, a lógica está diretamente em atualizarEstado.
    }

} // namespace LogisticSystem
