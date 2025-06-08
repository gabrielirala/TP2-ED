#include "entidades/Pacote.hpp"
#include <stdexcept> // Para std::out_of_range
#include <sstream>   // Para obterDetalhes

namespace LogisticSystem {

    Pacote::Pacote(ID_t id, Timestamp_t postagem, const std::string& rem,
                   const std::string& dest, const std::string& tipoPacote,
                   ID_t origem, ID_t destino)
        : idUnico(id), dataPostagem(postagem), remetente(rem),
          destinatario(dest), tipo(tipoPacote),
          armazemOrigem(origem), armazemDestino(destino),
          posicaoAtualRota(0), estadoAtual(EstadoPacote::NAO_POSTADO),
          tempoTotalArmazenado(0.0), tempoTotalTransito(0.0),
          timestampUltimaMudanca(postagem), tempoEsperadoTotal(0.0) {
        // O estado inicial é "NAO_POSTADO" e será atualizado para "CHEGADA_ESCALONADA"
        // ou "ARMAZENADO" pelo simulador ao agendar o evento de chegada inicial.
        historico.emplace_back(EstadoPacote::NAO_POSTADO, postagem, 0, "Pacote criado no sistema.");
    }

    void Pacote::definirRota(const std::vector<ID_t>& novaRota) {
        if (novaRota.empty()) {
            throw std::invalid_argument("A rota nao pode ser vazia.");
        }
        if (novaRota.front() != armazemOrigem || novaRota.back() != armazemDestino) {
            // Isso pode ser uma validação mais flexível dependendo da lógica do simulador.
            // Para rotas intermediárias, a origem e destino do pacote podem não corresponder
            // ao início e fim da rota atual.
            // Aqui, estamos assumindo que esta é a rota COMPLETA do pacote.
            // Se for uma sub-rota, a validação deve ser diferente.
            // throw std::invalid_argument("Rota invalida: O inicio ou fim da rota nao corresponde a origem/destino do pacote.");
        }
        rota = novaRota;
        posicaoAtualRota = 0;
        // Tempo esperado pode ser calculado com base na rota e tempos de transporte
        // mas isso provavelmente será feito pelo Simulador/SistemaTransporte.
    }

    ID_t Pacote::obterProximoArmazem() const {
        if (chegouDestino()) {
            return armazemDestino; // Já chegou ao destino final
        }
        if (posicaoAtualRota + 1 < rota.size()) {
            return rota[posicaoAtualRota + 1];
        }
        // Se a posição atual é a última na rota e não é o destino final
        // ou se a rota está vazia, o comportamento depende da lógica.
        // Para este protótipo, podemos retornar 0 ou lançar exceção.
        return 0; // Indicador de erro ou não há próximo armazém
    }

    bool Pacote::chegouDestino() const {
        if (rota.empty()) { // Se não há rota, ele só "chega" se a origem for o destino.
            return armazemOrigem == armazemDestino;
        }
        return (posicaoAtualRota >= rota.size() - 1) && (rota.back() == armazemDestino);
    }

    void Pacote::avancarNaRota() {
        if (!chegouDestino()) {
            posicaoAtualRota++;
        }
    }

    void Pacote::atualizarEstado(EstadoPacote novoEstado, Timestamp_t timestamp,
                               ID_t armazemId, const std::string& observacoes) {
        if (timestamp < timestampUltimaMudanca) {
            // Isso pode ser um erro, dependendo da precisão da simulação.
            // Para simplificar, podemos ignorar ou lançar uma exceção.
        }

        atualizarEstatisticasInternas(timestamp); // Atualiza métricas baseadas no tempo anterior

        estadoAtual = novoEstado;
        historico.emplace_back(novoEstado, timestamp, armazemId, observacoes);
        timestampUltimaMudanca = timestamp;

        // Notificar observadores sobre a mudança de estado
        notificarObservadores(historico.back());
    }

    void Pacote::atualizarEstatisticasInternas(Timestamp_t novoTimestamp) {
        // Calcula o tempo que o pacote passou no estado anterior
        if (timestampUltimaMudanca > 0 && novoTimestamp > timestampUltimaMudanca) {
            Timestamp_t tempoDecorrido = novoTimestamp - timestampUltimaMudanca;
            if (estadoAtual == EstadoPacote::ARMAZENADO || estadoAtual == EstadoPacote::CHEGOU_NAO_ARMAZENADO) {
                tempoTotalArmazenado += tempoDecorrido;
            } else if (estadoAtual == EstadoPacote::ALOCADO_TRANSPORTE) {
                // ALOCADO_TRANSPORTE é o estado *antes* de entrar no transporte.
                // O tempo em trânsito será contabilizado no EventoTransporte ou EventoChegada
                // quando o pacote efetivamente se move.
            } else if (estadoAtual == EstadoPacote::CHEGADA_ESCALONADA) {
                // Pacote esperando para ser postado/processado na origem.
                // Pode ser considerado tempo de espera ou tempo de processamento inicial.
            }
        }
    }

    MetricasPacote Pacote::calcularMetricas() const {
        MetricasPacote metricas;
        metricas.tempoEsperado = tempoEsperadoTotal; // Valor que deve ser definido externamente
        metricas.tempoArmazenado = tempoTotalArmazenado;
        metricas.tempoTransito = tempoTotalTransito; // Pode ser acumulado no evento de transporte

        // Cálculo de atraso total: assumindo que tempoEsperadoTotal é o target.
        // Atraso só faz sentido se o pacote foi entregue e o tempo de entrega é maior que o esperado.
        if (estadoAtual == EstadoPacote::ENTREGUE) {
            Timestamp_t tempoEntregaReal = historico.back().timestamp - dataPostagem;
            metricas.atrasoTotal = std::max(0.0, tempoEntregaReal - tempoEsperadoTotal);
        } else {
            // Se não entregue, o atraso ainda não é final. Pode ser um atraso parcial/acumulado.
            // Para simplificar, consideramos 0 se não foi entregue.
            metricas.atrasoTotal = 0.0;
        }

        metricas.numeroTransferencias = 0;
        for (size_t i = 0; i < historico.size(); ++i) {
            if (historico[i].estado == EstadoPacote::ALOCADO_TRANSPORTE && i + 1 < historico.size() &&
                historico[i+1].estado == EstadoPacote::CHEGADA_ESCALONADA) { // Chegada no próximo armazém
                metricas.numeroTransferencias++;
            }
        }
        metricas.gargaloDetectado = false; // Deve ser definido por um sistema de monitoramento/análise.

        return metricas;
    }

} // namespace LogisticSystem