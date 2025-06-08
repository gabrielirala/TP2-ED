#include "core/Escalonador.hpp"
#include <iostream> // Para log no modo debug
#include <chrono>   // Para medir tempo de processamento

namespace LogisticSystem {

    Escalonador::Escalonador()
        : relogioSimulacao(0.0), simulacaoAtiva(true), modoDebug(false) {
        // Inicializa estatísticas
        estatisticas = EstatisticasEscalonador();
    }

    void Escalonador::agendarEvento(std::shared_ptr<Evento> evento) {
        if (evento->obterTimestamp() < relogioSimulacao) {
            // Ignorar eventos agendados para o passado, ou tratar como erro
            if (modoDebug) {
                std::cerr << "[DEBUG] Evento " << evento->obterDetalhes()
                          << " agendado para o passado (" << evento->obterTimestamp()
                          << ") no tempo atual " << relogioSimulacao << ". Descartado." << std::endl;
            }
            estatisticas.eventosDescartados++;
            return;
        }
        filaEventos.inserir(evento);
        if (modoDebug) {
            logEvento(evento, "AGENDADO");
        }
    }

    std::shared_ptr<Evento> Escalonador::obterProximoEvento() {
        if (filaEventos.vazia()) {
            return nullptr;
        }
        return filaEventos.topo(); // Apenas olha, não remove
    }

    void Escalonador::removerEvento(std::shared_ptr<Evento> evento) {
        // A remoção de um evento específico que não seja o topo
        // em uma fila de prioridade pode ser ineficiente.
        // Para uma simulação discreta, geralmente removemos apenas o topo.
        // Se a remoção arbitrária for realmente necessária,
        // a FilaPrioridade precisaria de uma implementação mais complexa (heap com handles).
        // Por enquanto, vamos considerar que só removemos o topo.
        // Se for para remover qualquer evento, a implementação de FilaPrioridade
        // em ListaLigada pode ter um método remover(predicado).
        // Isso seria:
        // filaEventos.remover([&](const std::shared_ptr<Evento>& e){ return e == evento; });
        // Para este protótipo, vamos assumir que não é uma operação comum para eventos arbitrários.
    }

    void Escalonador::avancarRelogio(Timestamp_t novoTempo) {
        if (novoTempo < relogioSimulacao) {
            // Não deve acontecer em simulações forward
            if (modoDebug) {
                std::cerr << "[ERRO] Tentativa de avançar o relogio para o passado: "
                          << novoTempo << " < " << relogioSimulacao << std::endl;
            }
            return;
        }
        relogioSimulacao = novoTempo;
    }

    void Escalonador::reiniciarSimulacao() {
        filaEventos.limpar();
        relogioSimulacao = 0.0;
        simulacaoAtiva = true;
        estatisticas = EstatisticasEscalonador(); // Resetar estatísticas
        if (modoDebug) {
            std::cout << "[DEBUG] Escalonador reiniciado." << std::endl;
        }
    }

    void Escalonador::registrarProcessador(TipoEvento tipo, std::shared_ptr<IProcessadorEvento> processador) {
        // Evitar registrar o mesmo processador duas vezes para o mesmo tipo
        for (const auto& p : processadores[tipo]) {
            if (p == processador) {
                return;
            }
        }
        processadores[tipo].push_back(processador);
        if (modoDebug) {
            std::cout << "[DEBUG] Processador de evento registrado para o tipo: "
                      << static_cast<int>(tipo) << std::endl;
        }
    }

    void Escalonador::removerProcessador(TipoEvento tipo, std::shared_ptr<IProcessadorEvento> processador) {
        auto& lista = processadores[tipo];
        lista.erase(std::remove_if(lista.begin(), lista.end(),
                                   [processador](const std::shared_ptr<IProcessadorEvento>& p){
                                       return p == processador;
                                   }),
                    lista.end());
        if (modoDebug) {
            std::cout << "[DEBUG] Processador de evento removido para o tipo: "
                      << static_cast<int>(tipo) << std::endl;
        }
    }

    std::shared_ptr<Evento> Escalonador::processarProximoEvento() {
        if (!temEventosPendentes() || !simulacaoAtiva) {
            return nullptr;
        }

        auto evento = filaEventos.remover();
        avancarRelogio(evento->obterTimestamp());

        if (modoDebug) {
            logEvento(evento, "PROCESSANDO");
        }

        auto start_time = std::chrono::high_resolution_clock::now();

        // Encontrar e executar o processador apropriado
        bool processado = false;
        if (processadores.count(evento->obterTipo())) {
            for (auto& proc : processadores[evento->obterTipo()]) {
                if (proc->podeProcessar(evento->obterTipo())) {
                    proc->processarEvento(evento);
                    processado = true;
                    break; // Apenas um processador para o mesmo evento por enquanto
                }
            }
        }

        if (!processado) {
            if (modoDebug) {
                std::cerr << "[ALERTA] Nenhum processador encontrado para o evento: "
                          << evento->obterDetalhes() << std::endl;
            }
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        double tempoProcessamento = std::chrono::duration<double>(end_time - start_time).count();
        atualizarEstatisticas(evento->obterTipo(), tempoProcessamento);

        if (modoDebug) {
            logEvento(evento, "CONCLUIDO");
        }
        return evento;
    }

    void Escalonador::executarAteTimestamp(Timestamp_t limite) {
        if (modoDebug) {
            std::cout << "[DEBUG] Executando ate o timestamp: " << limite << std::endl;
        }
        while (temEventosPendentes() && simulacaoAtiva && obterTempoAtual() < limite) {
            auto proximo_evento = filaEventos.topo();
            if (proximo_evento->obterTimestamp() > limite) {
                if (modoDebug) {
                    std::cout << "[DEBUG] Proximo evento (" << proximo_evento->obterDetalhes()
                              << ") esta alem do limite. Parando execucao." << std::endl;
                }
                break;
            }
            processarProximoEvento();
        }
        if (modoDebug) {
            std::cout << "[DEBUG] Execucao ate timestamp " << limite << " finalizada." << std::endl;
        }
    }

    void Escalonador::executarNumeroEventos(int quantidade) {
        if (modoDebug) {
            std::cout << "[DEBUG] Executando " << quantidade << " eventos." << std::endl;
        }
        for (int i = 0; i < quantidade && temEventosPendentes() && simulacaoAtiva; ++i) {
            processarProximoEvento();
        }
        if (modoDebug) {
            std::cout << "[DEBUG] Execucao de " << quantidade << " eventos finalizada." << std::endl;
        }
    }

    void Escalonador::atualizarEstatisticas(TipoEvento tipo, Timestamp_t tempoProcessamento) {
        estatisticas.eventosProcessados++;
        estatisticas.tempoSimulacaoTotal = relogioSimulacao; // O tempo atual do relógio
        estatisticas.tempoMedioProcessamento =
            (estatisticas.tempoMedioProcessamento * (estatisticas.eventosProcessados - 1) + tempoProcessamento) / estatisticas.eventosProcessados;

        switch (tipo) {
            case TipoEvento::CHEGADA_PACOTE:
                estatisticas.eventosChegada++;
                break;
            case TipoEvento::TRANSPORTE:
                estatisticas.eventosTransporte++;
                break;
            case TipoEvento::MANUTENCAO:
                // estatisticas.eventosManutencao++; (se houver essa estatística)
                break;
            default:
                break;
        }
    }

    void Escalonador::logEvento(const std::shared_ptr<Evento>& evento, const std::string& acao) const {
        if (modoDebug) {
            std::cout << "[DEBUG] Tempo: " << relogioSimulacao
                      << " | Acao: " << acao
                      << " | Evento: " << evento->obterDetalhes()
                      << " (Prio: " << evento->obterPrioridade() << ")" << std::endl;
        }
    }

    void Escalonador::definirCapacidadeInicial(size_t capacidade) {
        // A FilaPrioridade baseada em ListaLigada não tem uma capacidade inicial fixada
        // como uma array-based heap. Este método pode ser usado para pré-alocar memória
        // se a fila de prioridade fosse implementada de outra forma, ou para validar
        // alguma expectativa de tamanho máximo.
        if (modoDebug) {
            std::cout << "[DEBUG] Capacidade inicial do escalonador definida (se aplicavel): "
                      << capacidade << std::endl;
        }
    }

    void Escalonador::limpar() {
        filaEventos.limpar();
        relogioSimulacao = 0.0;
        simulacaoAtiva = true; // Ou false, dependendo da semântica de "limpar"
        estatisticas = EstatisticasEscalonador();
        processadores.clear();
        if (modoDebug) {
            std::cout << "[DEBUG] Escalonador limpo." << std::endl;
        }
    }

    std::vector<std::shared_ptr<Evento>> Escalonador::obterEventosPorTipo(TipoEvento tipo) const {
        std::vector<std::shared_ptr<Evento>> eventosFiltrados;
        // Iterar sobre a fila de prioridade para encontrar eventos do tipo
        // Isso pode ser ineficiente para grandes filas.
        // A FilaPrioridade aqui não expõe uma interface para iterar facilmente
        // sem remover elementos, então teríamos que adicionar um iterador ou
        // um método de acesso aos elementos internos para isso.
        // Por simplicidade, assumimos que esta funcionalidade pode não ser crítica
        // para o desempenho em um protótipo.
        // Se a FilaPrioridade fosse uma heap, seria ainda mais complicado.
        // Para uma lista ligada, poderíamos iterar.
        // Exemplo:
        // for (const auto& evento_ptr : filaEventos.obterTodosElementos()) {
        //     if (evento_ptr->obterTipo() == tipo) {
        //         eventosFiltrados.push_back(evento_ptr);
        //     }
        // }
        // Isso requer um método obterTodosElementos() na FilaPrioridade.
        // Como não temos esse método no FilaPrioridade.hpp, não podemos implementar agora.
        return eventosFiltrados;
    }

    std::vector<std::shared_ptr<Evento>> Escalonador::obterEventosAteTimestamp(Timestamp_t limite) const {
        std::vector<std::shared_ptr<Evento>> eventosFiltrados;
        // Similar ao método acima, requer um método para iterar sobre a fila sem removê-los.
        return eventosFiltrados;
    }

} // namespace LogisticSystem