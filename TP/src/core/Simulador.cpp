#include "core/Simulador.hpp"
#include "utils/LeitorArquivos.hpp"
#include "eventos/EventoChegada.hpp"
#include "eventos/EventoTransporte.hpp"
#include "eventos/EventoPostagem.hpp" // Pode não ser mais necessário se EventoChegada cobre
#include "eventos/EventoEntrega.hpp"   // Pode não ser mais necessário se EventoChegada cobre
#include "estruturas/Grafo.hpp"        // Para buscar rota
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <limits> // Para std::numeric_limits

namespace LogisticSystem {

    Simulador::Simulador()
        : simulacaoInicializada(false), simulacaoFinalizada(false) {
        // Inicializar os componentes principais com unique_ptr
        escalonador = std::make_unique<Escalonador>();
        redeArmazens = std::make_unique<RedeArmazens>();
        // SistemaTransporte precisa do Grafo da RedeArmazens, que é criado dentro de RedeArmazens
        sistemaTransporte = std::make_unique<SistemaTransporte>(redeArmazens->obterGrafo());
        // O gerenciador de estatísticas será inicializado após carregar os pacotes e configurar os outros componentes.
        gerenciadorEstatisticas = nullptr; // Inicializa como nulo, será criado no inicializar
    }

    Simulador::~Simulador() {
        limparRecursos();
    }

    // Inicializa a simulação lendo de um único arquivo de entrada
    bool Simulador::inicializar(const std::string& arquivoEntrada) {
        if (simulacaoInicializada) {
            std::cout << "Simulacao ja inicializada. Reinicializando..." << std::endl;
            reinicializar();
        }

        // 1. Carregar parâmetros globais e topologia usando a nova função do LeitorArquivos
        try {
            // A função lerConfiguracaoESetup preenche redeArmazens e sistemaTransporte
            parametrosSimulacao = LeitorArquivos::lerConfiguracaoESetup(
                arquivoEntrada,
                std::shared_ptr<RedeArmazens>(redeArmazens.get(), [](RedeArmazens*){}), // Passa shared_ptr do raw pointer
                std::shared_ptr<SistemaTransporte>(sistemaTransporte.get(), [](SistemaTransporte*){}) // Passa shared_ptr do raw pointer
            );
            // Definir tempo final da simulação e arquivo de saída a partir dos parâmetros lidos
            // (Assumindo que ParametrosSimulacaoGlobal agora contém esses campos)
            parametrosSimulacao.tempoFinalSimulacao = 10000.0; // Valor dummy, você pode ler de algum lugar se necessário
            parametrosSimulacao.modoDebug = true; // Valor dummy
            parametrosSimulacao.arquivoSaida = "resultados/relatorio_simulacao.txt"; // Valor dummy
            escalonador->habilitarModoDebug(parametrosSimulacao.modoDebug);

        } catch (const std::exception& e) {
            std::cerr << "Falha ao carregar configuracao e topologia: " << e.what() << std::endl;
            return false;
        }

        // 2. Carregar pacotes
        try {
            int numPacotesLidos; // Para capturar o número de pacotes lidos
            pacotes = LeitorArquivos::lerPacotes(arquivoEntrada, numPacotesLidos);
            if (pacotes.empty()) {
                std::cerr << "Nenhum pacote foi lido do arquivo de entrada." << std::endl;
                // return false; // Dependendo se simulação sem pacotes é válida
            }
        } catch (const std::exception& e) {
            std::cerr << "Falha ao carregar pacotes: " << e.what() << std::endl;
            return false;
        }

        // 3. Configurar sistemas (rotas dos pacotes, registro de processadores)
        configurarSistemas();

        // 4. Agendar eventos iniciais (postagem de pacotes, primeiros transportes)
        agendarEventosIniciais();

        // 5. Inicializar o gerenciador de estatísticas após todos os componentes estarem prontos
        gerenciadorEstatisticas = std::make_unique<GerenciadorEstatisticas>(
            std::shared_ptr<Escalonador>(escalonador.get(), [](Escalonador*){}),
            std::shared_ptr<RedeArmazens>(redeArmazens.get(), [](RedeArmazens*){}),
            std::shared_ptr<SistemaTransporte>(sistemaTransporte.get(), [](SistemaTransporte*){}),
            pacotes // Passa a lista de pacotes
        );

        if (!validarConsistencia()) {
            std::cerr << "Falha na validacao de consistencia da rede ou pacotes." << std::endl;
            return false;
        }

        simulacaoInicializada = true;
        simulacaoFinalizada = false;
        std::cout << "Simulacao inicializada com sucesso." << std::endl;
        return true;
    }

    void Simulador::reinicializar() {
        if (escalonador) escalonador->reiniciarSimulacao();
        if (redeArmazens) redeArmazens->limpar();
        if (sistemaTransporte) sistemaTransporte->limpar(); // Assumindo um método limpar em SistemaTransporte
        pacotes.clear();
        gerenciadorEstatisticas.reset(); // Reinicia o gerenciador de estatísticas
        simulacaoInicializada = false;
        simulacaoFinalizada = false;
        std::cout << "Simulador reinicializado." << std::endl;
    }

    bool Simulador::executarSimulacao() {
        if (!simulacaoInicializada) {
            std::cerr << "Erro: Simulacao nao inicializada." << std::endl;
            return false;
        }

        std::cout << "Iniciando execucao da simulacao..." << std::endl;
        while (escalonador->temEventosPendentes() &&
               escalonador->obterTempoAtual() < parametrosSimulacao.tempoFinalSimulacao &&
               !simulacaoFinalizada) {
            escalonador->processarProximoEvento();
            if (escalonador->obterTempoAtual() >= parametrosSimulacao.tempoFinalSimulacao) {
                std::cout << "Tempo final (" << parametrosSimulacao.tempoFinalSimulacao << ") atingido." << std::endl;
                break;
            }
            if (verificarCondicaoFinalizacao()) {
                std::cout << "Condicao de finalizacao antecipada atingida." << std::endl;
                break;
            }
        }

        finalizarSimulacao();
        return true;
    }

    void Simulador::executarAteTimestamp(Timestamp_t limite) {
        if (!simulacaoInicializada) {
            std::cerr << "Erro: Simulacao nao inicializada." << std::endl;
            return;
        }
        if (limite < escalonador->obterTempoAtual()) {
             std::cerr << "Erro: Limite de tempo invalido (passado)." << std::endl;
             return;
        }
        escalonador->executarAteTimestamp(limite);
        if (verificarCondicaoFinalizacao()) {
            finalizarSimulacao();
        }
    }

    void Simulador::executarProximoEvento() {
        if (!simulacaoInicializada) {
            std::cerr << "Erro: Simulacao nao inicializada." << std::endl;
            return;
        }
        if (escalonador->temEventosPendentes() &&
            escalonador->obterTempoAtual() < parametrosSimulacao.tempoFinalSimulacao &&
            !simulacaoFinalizada) {
            escalonador->processarProximoEvento();
        } else {
            std::cout << "Nenhum evento pendente ou tempo final atingido." << std::endl;
            finalizarSimulacao();
        }
    }

    void Simulador::pararSimulacao() {
        escalonador->pararSimulacao();
        simulacaoFinalizada = true;
        std::cout << "Simulacao interrompida." << std::endl;
    }

    void Simulador::processarEvento(std::shared_ptr<Evento> evento) {
        if (parametrosSimulacao.modoDebug) {
            std::cout << "[Simulador] Tempo: " << escalonador->obterTempoAtual()
                      << " - Processando evento: " << evento->obterDetalhes() << std::endl;
        }

        // O Simulador delega o processamento real do evento às entidades.
        // Garante que o evento tenha suas dependências configuradas (Armazem, SistemaTransporte, Escalonador).
        if (evento->obterTipo() == TipoEvento::PACOTE) { // Antigo EventoChegada/Postagem
            auto chegada_evento = std::static_pointer_cast<EventoChegada>(evento); // Assumindo EventoChegada lida com chegada e postagem
            auto armazem_id = chegada_evento->obterArmazemDestino(); // ID do armazém onde o pacote chega
            chegada_evento->definirArmazem(redeArmazens->obterArmazem(armazem_id));
            chegada_evento->definirEscalonador(shared_from_this_as_Escalonador()); // Usando helper
            chegada_evento->definirRedeArmazens(shared_from_this_as_RedeArmazens()); // Para cálculo de rota, se necessário
            chegada_evento->definirSimulador(shared_from_this()); // Para acesso ao simulador completo
        } else if (evento->obterTipo() == TipoEvento::TRANSPORTE) {
            auto transporte_evento = std::static_pointer_cast<EventoTransporte>(evento);
            auto origem_id = transporte_evento->obterArmazemOrigem();
            auto destino_id = transporte_evento->obterArmazemDestino();
            transporte_evento->definirArmazemOrigem(redeArmazens->obterArmazem(origem_id));
            transporte_evento->definirArmazemDestino(redeArmazens->obterArmazem(destino_id));
            transporte_evento->definirSistemaTransporte(shared_from_this_as_SistemaTransporte()); // Usando helper
            transporte_evento->definirEscalonador(shared_from_this_as_Escalonador()); // Usando helper
            transporte_evento->definirSimulador(shared_from_this()); // Para acesso ao simulador completo
        }

        evento->executar();
    }

    // Funções auxiliares para obter shared_ptr de membros unique_ptr
    std::shared_ptr<Escalonador> Simulador::shared_from_this_as_Escalonador() {
        return std::shared_ptr<Escalonador>(escalonador.get(), [](Escalonador*){});
    }

    std::shared_ptr<RedeArmazens> Simulador::shared_from_this_as_RedeArmazens() {
        return std::shared_ptr<RedeArmazens>(redeArmazens.get(), [](RedeArmazens*){});
    }

    std::shared_ptr<SistemaTransporte> Simulador::shared_from_this_as_SistemaTransporte() {
        return std::shared_ptr<SistemaTransporte>(sistemaTransporte.get(), [](SistemaTransporte*){});
    }

    std::shared_ptr<GerenciadorEstatisticas> Simulador::shared_from_this_as_GerenciadorEstatisticas() {
        return std::shared_ptr<GerenciadorEstatisticas>(gerenciadorEstatisticas.get(), [](GerenciadorEstatisticas*){});
    }

    bool Simulador::podeProcessar(TipoEvento tipo) const {
        // O simulador pode "processar" todos os tipos de evento, pois ele os delega.
        return (tipo == TipoEvento::PACOTE || tipo == TipoEvento::TRANSPORTE);
    }

    Timestamp_t Simulador::obterTempoAtual() const {
        return escalonador->obterTempoAtual();
    }

    void Simulador::gerarRelatorios() const {
        if (!simulacaoFinalizada) {
            std::cerr << "A simulacao nao esta finalizada. Nao eh possivel gerar relatorios completos." << std::endl;
            return;
        }
        if (gerenciadorEstatisticas) {
            gerenciadorEstatisticas->gerarRelatorioTexto(parametrosSimulacao.arquivoSaida);
        } else {
            std::cerr << "Gerenciador de estatisticas nao inicializado." << std::endl;
        }
    }

    void Simulador::salvarEstatisticas() const {
        // Exemplo: salvarEstatisticas(parametrosSimulacao.arquivoSaida + ".bin");
        std::cerr << "Funcionalidade de salvar estatisticas binarias nao implementada." << std::endl;
    }

    void Simulador::salvarEstatisticas(const std::string& arquivo) const {
        if (!simulacaoFinalizada) {
            std::cerr << "A simulacao nao esta finalizada. Nao eh possivel salvar estatisticas completas." << std::endl;
            return;
        }
        if (gerenciadorEstatisticas) {
            gerenciadorEstatisticas->salvarRelatorioBinario(arquivo); // Implementação dummy por enquanto
        } else {
            std::cerr << "Gerenciador de estatisticas nao inicializado." << std::endl;
        }
    }

    void Simulador::configurarSistemas() {
        // Registra o Simulador como processador de eventos no Escalonador.
        // `shared_from_this()` retorna um shared_ptr para a instância atual do Simulador.
        escalonador->registrarProcessador(TipoEvento::PACOTE, shared_from_this());
        escalonador->registrarProcessador(TipoEvento::TRANSPORTE, shared_from_this());

        // Para cada pacote lido, calcular sua rota.
        for (const auto& pacote : pacotes) {
            // A rota deve ser calculada usando BFS no Grafo da RedeArmazens
            // como o enunciado exige para grafos não ponderados (assumindo que o grafo
            // foi configurado para ser não ponderado e BFS é mais adequado).
            // Seu `Grafo::buscarMenorCaminho` deve ser adaptado para BFS.
            ListaLigada<ID_t> rota = redeArmazens->obterGrafo()->buscarMenorCaminho(
                pacote->obterArmazemOrigem(), pacote->obterArmazemDestino());
            pacote->definirRota(rota);
        }
    }

    void Simulador::agendarEventosIniciais() {
        // Agendar eventos de chegada iniciais para todos os pacotes (postagem)
        for (const auto& pacote : pacotes) {
            // EventoChegada agora pode lidar com postagem
            auto eventoChegada = std::make_shared<EventoChegada>(
                pacote->obterDataPostagem(), // Timestamp do evento
                pacote->obterArmazemOrigem(),// Armazem onde o pacote chega/e postado
                pacote // Pacote associado
            );
            // Definir dependências do evento (passando shared_ptr para evitar problemas de ownership)
            eventoChegada->definirArmazem(redeArmazens->obterArmazem(pacote->obterArmazemOrigem()));
            eventoChegada->definirEscalonador(shared_from_this_as_Escalonador());
            eventoChegada->definirSimulador(shared_from_this()); // Passa referência para o simulador
            escalonador->agendarEvento(eventoChegada);
        }

        // Agendar os primeiros eventos de transporte para todas as rotas
        // A frequência é dada por intervaloTransportesGlobal
        sistemaTransporte->agendarTransportesIniciais(
            escalonador, parametrosSimulacao.intervaloTransportesGlobal);

        // A `agendarTransportesIniciais` do SistemaTransporte deve criar e agendar os EventoTransporte
        // com o escalonador e a frequência correta.
        // Exemplo:
        // for (const auto& rota_pair : sistemaTransporte->obterTodasRotas()) {
        //     auto rota = sistemaTransporte->obterRota(rota_pair.first, rota_pair.second);
        //     if (rota) {
        //         // O EventoTransporte precisa ser agendado no tempo apropriado
        //         // (por exemplo, tempo atual + intervaloTransportesGlobal)
        //         auto eventoTransporte = std::make_shared<EventoTransporte>(
        //             rota->obterOrigem(),
        //             rota->obterDestino(),
        //             escalonador->obterTempoAtual() + parametrosSimulacao.intervaloTransportesGlobal // Próximo agendamento
        //         );
        //         eventoTransporte->definirArmazemOrigem(redeArmazens->obterArmazem(rota->obterOrigem()));
        //         eventoTransporte->definirArmazemDestino(redeArmazens->obterArmazem(rota->obterDestino()));
        //         eventoTransporte->definirSistemaTransporte(shared_from_this_as_SistemaTransporte());
        //         eventoTransporte->definirEscalonador(shared_from_this_as_Escalonador());
        //         eventoTransporte->definirSimulador(shared_from_this());
        //         escalonador->agendarEvento(eventoTransporte);
        //     }
        // }
    }


    bool Simulador::verificarCondicaoFinalizacao() {
        // Critério de finalização:
        // 1. O tempo final da simulação foi atingido (já verificado no loop principal)
        // 2. Não há mais eventos pendentes no escalonador
        // 3. Todos os pacotes foram entregues (ou estão em um estado final)

        if (!escalonador->temEventosPendentes()) {
            std::cout << "Simulacao finalizada: Nao ha mais eventos pendentes." << std::endl;
            return true;
        }

        bool todos_pacotes_entregues = true;
        for (const auto& pacote : pacotes) {
            if (pacote->obterEstadoAtual() != EstadoPacote::ENTREGUE) {
                todos_pacotes_entregues = false;
                break;
            }
        }
        if (todos_pacotes_entregues) {
            std::cout << "Simulacao finalizada: Todos os pacotes foram entregues." << std::endl;
            return true;
        }

        return false;
    }

    void Simulador::finalizarSimulacao() {
        if (simulacaoFinalizada) return;

        std::cout << "Finalizando simulacao..." << std::endl;
        simulacaoFinalizada = true;
        escalonador->pararSimulacao(); // Garante que o escalonador pare.

        // Coletar estatísticas finais
        if (gerenciadorEstatisticas) {
            gerenciadorEstatisticas->coletarEstatisticas();
        }
        std::cout << "Simulacao concluida no tempo: " << escalonador->obterTempoAtual() << std::endl;
    }

    void Simulador::limparRecursos() {
        // unique_ptr cuidarão da memória automaticamente quando saírem do escopo
        escalonador.reset();
        redeArmazens.reset();
        sistemaTransporte.reset();
        gerenciadorEstatisticas.reset();
        pacotes.clear();
        std::cout << "Recursos do simulador limpos." << std::endl;
    }

    bool Simulador::validarParametros() const {
        // Estes parâmetros agora vêm de ParametrosSimulacaoGlobal
        // Você pode adicionar mais validações aqui se necessário
        if (parametrosSimulacao.latenciaTransporteGlobal <= 0) {
            std::cerr << "Erro de Parametro: latenciaTransporteGlobal deve ser positiva." << std::endl;
            return false;
        }
        if (parametrosSimulacao.custoRemocaoGlobal <= 0) {
             std::cerr << "Erro de Parametro: custoRemocaoGlobal deve ser positivo." << std::endl;
             return false;
        }
        if (parametrosSimulacao.intervaloTransportesGlobal <= 0) {
             std::cerr << "Erro de Parametro: intervaloTransportesGlobal deve ser positivo." << std::endl;
             return false;
        }
        if (parametrosSimulacao.capacidadeTransporteGlobal <= 0) {
             std::cerr << "Erro de Parametro: capacidadeTransporteGlobal deve ser positiva." << std::endl;
             return false;
        }
        return true;
    }

    bool Simulador::validarArquivos() const {
        // Agora, validação é para um único arquivo de entrada
        std::ifstream entrada_file(parametrosSimulacao.arquivoSaida); // Usando arquivoSaida para o nome do arquivo de entrada
        if (!entrada_file.good()) {
            std::cerr << "Erro: Arquivo de entrada nao encontrado ou inacessivel: " << parametrosSimulacao.arquivoSaida << std::endl;
            return false;
        }
        return true;
    }

    bool Simulador::validarConsistencia() const {
        // Verificar se os IDs de origem/destino de pacotes e rotas existem na rede de armazéns
        for (const auto& pacote : pacotes) {
            if (!redeArmazens->obterArmazem(pacote->obterArmazemOrigem())) {
                std::cerr << "Consistencia Falha: Armazem de origem do pacote " << pacote->obterIdUnico() << " (" << pacote->obterArmazemOrigem() << ") nao existe." << std::endl;
                return false;
            }
            if (!redeArmazens->obterArmazem(pacote->obterArmazemDestino())) {
                std::cerr << "Consistencia Falha: Armazem de destino do pacote " << pacote->obterIdUnico() << " (" << pacote->obterArmazemDestino() << ") nao existe." << std::endl;
                return false;
            }
        }

        auto todas_rotas = sistemaTransporte->obterTodasRotas();
        for (const auto& rota_pair : todas_rotas) {
            if (!redeArmazens->obterArmazem(rota_pair.first)) {
                std::cerr << "Consistencia Falha: Armazem de origem da rota " << rota_pair.first << "->" << rota_pair.second << " nao existe." << std::endl;
                return false;
            }
            if (!redeArmazens->obterArmazem(rota_pair.second)) {
                std::cerr << "Consistencia Falha: Armazem de destino da rota " << rota_pair.first << "->" << rota_pair.second << " nao existe." << std::endl;
                return false;
            }
            // Verificar se o armazém de origem tem uma seção para o armazém de destino da rota
            if (!redeArmazens->obterArmazem(rota_pair.first)->temSecao(rota_pair.second)) {
                 std::cerr << "Consistencia Falha: Armazem " << rota_pair.first << " nao possui secao para o destino " << rota_pair.second << "." << std::endl;
                 return false;
            }
        }
        return true;
    }

} // namespace LogisticSystem
