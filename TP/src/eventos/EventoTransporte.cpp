#include "eventos/EventoTransporte.hpp"
#include "entidades/Armazem.hpp"
#include "core/Escalonador.hpp"
#include <sstream> // Para obterDetalhes
#include <iostream> // Para debug

namespace LogisticSystem {

    EventoTransporte::EventoTransporte(ID_t origem, ID_t destino, Timestamp_t tempo)
        : Evento(TipoEvento::TRANSPORTE, tempo, 0), // Prioridade 0, pois pode ser menos prioritário que chegadas
          armazemOrigem(origem), armazemDestino(destino) {
        descricao = "Transporte de " + std::to_string(origem) + " para " + std::to_string(destino) +
                    " agendado para o tempo " + std::to_string(tempo);
    }

    void EventoTransporte::executar() {
        if (!validarPrecondições()) {
            std::cerr << "Erro: Precondicoes para EventoTransporte nao satisfeitas." << std::endl;
            return;
        }

        auto sistemaTransporte_ptr = sistemaTransporte.lock();
        auto armazemOrigem_ptr = armazemOrigemRef.lock();
        auto escalonador_ptr = escalonador.lock();

        if (!sistemaTransporte_ptr || !armazemOrigem_ptr || !escalonador_ptr) {
            std::cerr << "Erro: Componentes essenciais indisponiveis para o EventoTransporte." << std::endl;
            return;
        }

        // Verifica se o transporte pode ser executado no tempo atual.
        if (!sistemaTransporte_ptr->podeExecutarTransporte(armazemOrigem, armazemDestino, timestamp)) {
            // Este transporte ainda não pode ocorrer (ex: tempo mínimo não atingido).
            // Reagendar para o próximo tempo válido ou logar um aviso.
            std::cerr << "AVISO: Tentativa de executar transporte fora do agendamento para "
                      << armazemOrigem << "->" << armazemDestino << " no tempo " << timestamp << std::endl;
            // Pode ser reagendado automaticamente pelo simulador ou sistema de transporte
            sistemaTransporte_ptr->agendarProximoTransporte(armazemOrigem, armazemDestino, timestamp);
            return;
        }

        executarTransporte();
        agendarProximoTransporte(); // Agenda a próxima ocorrência deste transporte
    }

    void EventoTransporte::executarTransporte() {
        auto armazemOrigem_ptr = armazemOrigemRef.lock();
        auto sistemaTransporte_ptr = sistemaTransporte.lock();
        auto escalonador_ptr = escalonador.lock();
        auto armazemDestino_ptr = armazemDestinoRef.lock(); // Necessário para configurar o EventoChegada

        if (!armazemOrigem_ptr || !sistemaTransporte_ptr || !escalonador_ptr || !armazemDestino_ptr) {
            std::cerr << "Erro critico: Componentes ausentes durante a execucao do transporte." << std::endl;
            return;
        }

        // Obtém a capacidade máxima da rota
        const auto* rota = sistemaTransporte_ptr->obterRota(armazemOrigem, armazemDestino);
        if (!rota) {
            std::cerr << "Erro: Rota " << armazemOrigem << "->" << armazemDestino << " nao encontrada." << std::endl;
            return;
        }
        Capacity_t capacidade_transporte = rota->obterCapacidadeMaxima();

        // Prepara os pacotes para transporte no armazém de origem
        std::vector<std::shared_ptr<Pacote>> pacotes_a_transportar =
            armazemOrigem_ptr->prepararTransporte(armazemDestino, capacidade_transporte, timestamp);

        if (pacotes_a_transportar.empty()) {
            // Não há pacotes para transportar nesta rota neste momento.
            // Isso não é um erro, apenas significa que o transporte ocorreu vazio.
            std::cout << "DEBUG: Transporte de " << armazemOrigem << " para " << armazemDestino
                      << " executado vazio no tempo " << timestamp << std::endl;
            sistemaTransporte_ptr->registrarTransporteExecutado(armazemOrigem, armazemDestino, 0, timestamp);
            return;
        }

        // Calcula o tempo de chegada ao destino
        Timestamp_t tempo_chegada = sistemaTransporte_ptr->calcularTempoChegada(armazemOrigem, armazemDestino, timestamp);

        // Agendar eventos de chegada para cada pacote transportado
        agendarChegadas(pacotes_a_transportar);

        // Registrar a viagem no sistema de transporte para estatísticas
        sistemaTransporte_ptr->registrarTransporteExecutado(armazemOrigem, armazemDestino,
                                                           (Capacity_t)pacotes_a_transportar.size(), timestamp);

        std::cout << "DEBUG: Transporte de " << armazemOrigem << " para " << armazemDestino
                  << " com " << pacotes_a_transportar.size() << " pacotes no tempo " << timestamp << std::endl;
    }

    void EventoTransporte::agendarChegadas(const std::vector<std::shared_ptr<Pacote>>& pacotes) {
        auto escalonador_ptr = escalonador.lock();
        auto armazemDestino_ptr = armazemDestinoRef.lock();
        auto sistemaTransporte_ptr = sistemaTransporte.lock();

        if (!escalonador_ptr || !armazemDestino_ptr || !sistemaTransporte_ptr) {
            std::cerr << "Erro: Nao foi possivel agendar chegadas. Componentes ausentes." << std::endl;
            return;
        }

        Timestamp_t tempo_chegada_base = sistemaTransporte_ptr->calcularTempoChegada(armazemOrigem, armazemDestino, timestamp);

        for (const auto& pacote_ptr : pacotes) {
            // Cada pacote avança na rota após ser transportado.
            pacote_ptr->avancarNaRota();
            // Calcula o tempo que o pacote passou em trânsito (aqui ou no EventoChegada)
            // Pacote em transito significa estado ALOCADO_TRANSPORTE ate o CHEGADA_ESCALONADA.
            // Essa métrica é melhor no EventoChegada.

            auto eventoChegada = std::make_shared<EventoChegada>(pacote_ptr, armazemDestino, tempo_chegada_base);
            eventoChegada->definirArmazem(armazemDestino_ptr);
            eventoChegada->definirEscalonador(escalonador_ptr);
            escalonador_ptr->agendarEvento(eventoChegada);
        }
    }

    void EventoTransporte::agendarProximoTransporte() {
        auto sistemaTransporte_ptr = sistemaTransporte.lock();
        if (sistemaTransporte_ptr) {
            // A função `registrarTransporteExecutado` do SistemaTransporte já
            // agendará o próximo transporte para esta rota.
            // Então, não precisamos de uma chamada explícita aqui se essa for a lógica.
            // Ou, se o próximo agendamento for sempre `timestamp + intervaloTransporte`:
            // sistemaTransporte_ptr->agendarProximoTransporte(armazemOrigem, armazemDestino, timestamp);
            // Isso já é feito na execução do transporte.
        }
    }

    bool EventoTransporte::validarPrecondições() const {
        auto armazemOrigem_ptr = armazemOrigemRef.lock();
        auto armazemDestino_ptr = armazemDestinoRef.lock();
        auto sistemaTransporte_ptr = sistemaTransporte.lock();
        auto escalonador_ptr = escalonador.lock();

        if (!armazemOrigem_ptr || !armazemDestino_ptr || !sistemaTransporte_ptr || !escalonador_ptr) {
            std::cerr << "Validacao falhou: Um ou mais componentes essenciais (Armazem Origem/Destino, Sistema Transporte, Escalonador) sao nulos." << std::endl;
            return false;
        }

        if (!sistemaTransporte_ptr->existeRota(armazemOrigem, armazemDestino)) {
            std::cerr << "Validacao falhou: Rota de transporte " << armazemOrigem << "->" << armazemDestino << " nao existe." << std::endl;
            return false;
        }
        return true;
    }

    std::unique_ptr<Evento> EventoTransporte::clonar() const {
        return std::make_unique<EventoTransporte>(*this);
    }

    std::string EventoTransporte::obterDetalhes() const {
        std::stringstream ss;
        ss << "EventoTransporte (Origem: " << armazemOrigem
           << ", Destino: " << armazemDestino
           << ", Tempo: " << timestamp << ")";
        return ss.str();
    }

} // namespace LogisticSystem