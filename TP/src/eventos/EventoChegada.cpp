#include "eventos/EventoChegada.hpp"
#include "entidades/Armazem.hpp"
#include "core/Escalonador.hpp"
#include <sstream> // Para obterDetalhes
#include <iostream> // Para debug

namespace LogisticSystem {

    EventoChegada::EventoChegada(std::shared_ptr<Pacote> pct, ID_t destino, Timestamp_t tempo)
        : Evento(TipoEvento::CHEGADA_PACOTE, tempo, 1), pacote(pct), armazemDestino(destino) {
        descricao = "Chegada de pacote " + std::to_string(pacote->obterIdUnico()) +
                    " ao armazem " + std::to_string(destino);
    }

    void EventoChegada::executar() {
        if (!pacote) {
            std::cerr << "Erro: Pacote nulo no EventoChegada." << std::endl;
            return;
        }

        auto armazem_ptr = armazem.lock();
        if (!armazem_ptr) {
            std::cerr << "Erro: Armazem de destino indisponivel para o EventoChegada." << std::endl;
            return;
        }

        // Verifica se o pacote chegou ao seu destino final
        if (pacote->chegouDestino() && pacote->obterArmazemDestino() == armazem_ptr->obterIdArmazem()) {
            processarChegadaFinal();
        } else {
            // Chegada em um armazém intermediário
            processarChegadaIntermediaria();
            agendarProximoMovimento();
        }
    }

    void EventoChegada::processarChegadaFinal() {
        auto armazem_ptr = armazem.lock();
        if (armazem_ptr) {
            // O pacote já foi marcado como ENTREGUE na função Armazem::receberPacote
            // ou será marcado aqui se for uma chegada direta ao destino final sem passar por Armazem::receberPacote
            // Ex: Se o evento de chegada final não for gerado por Armazem::receberPacote.
            // Para manter a consistência, Armazem::receberPacote deve ser a única entrada para o pacote no armazém.
            // Se o pacote está em ALOCADO_TRANSPORTE e chega ao destino final, ele é ENTREGUE.
            if (pacote->obterEstadoAtual() != EstadoPacote::ENTREGUE) {
                pacote->atualizarEstado(EstadoPacote::ENTREGUE, timestamp, armazem_ptr->obterIdArmazem(), "Pacote entregue ao destino final.");
                // O armazém precisaria ser notificado sobre a saída do pacote, ou o simulador gerenciar o total.
            }
        }
    }

    void EventoChegada::processarChegadaIntermediaria() {
        auto armazem_ptr = armazem.lock();
        if (!armazem_ptr) {
            return;
        }

        // Antes de tentar armazenar, atualiza o estado para indicar chegada.
        // O estado pode ser "CHEGOU_NAO_ARMAZENADO" se o armazém estiver cheio.
        // A função `receberPacote` do armazém fará essa verificação.
        // O pacote pode ter sido ALOCADO_TRANSPORTE e agora CHEGADA_ESCALONADA
        // antes de ser ARMAZENADO.
        pacote->atualizarEstado(EstadoPacote::CHEGADA_ESCALONADA, timestamp, armazem_ptr->obterIdArmazem(), "Chegou em armazem intermediario.");

        // Tenta armazenar o pacote no armazém
        bool armazenado = armazem_ptr->receberPacote(pacote, timestamp);

        if (!armazenado) {
            // Se o pacote não pôde ser armazenado (armazém cheio, seção não encontrada),
            // o estado já foi atualizado para CHEGOU_NAO_ARMAZENADO dentro de Armazem::receberPacote
            std::cerr << "AVISO: Pacote " << pacote->obterIdUnico()
                      << " nao pode ser armazenado no armazem " << armazem_ptr->obterIdArmazem()
                      << ". Estado: " << static_cast<int>(pacote->obterEstadoAtual()) << std::endl;
            // Ação alternativa: Pacote pode ser descartado, reenviado, aguardar, etc.
            // Para o protótipo, ele simplesmente fica em CHEGOU_NAO_ARMAZENADO.
        } else {
            // Pacote foi armazenado com sucesso, estado já é ARMAZENADO.
            // O pacote foi avançado na rota dentro de Armazem::receberPacote, ou deveria ser.
            // Se a rota ainda nao foi avancada, faca aqui:
            // pacote->avancarNaRota(); // Isso deve ocorrer *após* a saída do armazém para o próximo trecho
        }
    }

    void EventoChegada::agendarProximoMovimento() {
        auto armazem_ptr = armazem.lock();
        auto escalonador_ptr = escalonador.lock();
        if (!armazem_ptr || !escalonador_ptr) {
            std::cerr << "Erro: Armazem ou Escalonador indisponivel para agendar proximo movimento." << std::endl;
            return;
        }

        // Após a chegada, o pacote está ARMAZENADO (ou CHEGOU_NAO_ARMAZENADO).
        // Ele aguardará um EventoTransporte para sair.
        // Não é o EventoChegada que agenda o próximo transporte diretamente,
        // mas sim o ciclo de transporte regular do SistemaTransporte
        // ou algum evento de otimização/despacho.

        // Portanto, aqui não há agendamento direto de outro evento para este pacote.
        // O pacote está agora "na posse" do armazém e será considerado para o próximo transporte
        // quando a rota correspondente for ativada pelo SistemaTransporte.
    }

    std::unique_ptr<Evento> EventoChegada::clonar() const {
        return std::make_unique<EventoChegada>(*this);
    }

    std::string EventoChegada::obterDetalhes() const {
        std::stringstream ss;
        ss << "EventoChegada (Pacote ID: " << pacote->obterIdUnico()
           << ", Destino: " << armazemDestino
           << ", Tempo: " << timestamp << ")";
        return ss.str();
    }

} // namespace LogisticSystem