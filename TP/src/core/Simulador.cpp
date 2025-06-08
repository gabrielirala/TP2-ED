#include "core/Simulador.hpp"
#include "utils/LeitorArquivos.hpp" // Para ler topologia e pacotes
#include "eventos/EventoChegada.hpp"
#include "eventos/EventoTransporte.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>

namespace LogisticSystem {

    Simulador::Simulador()
        : simulacaoInicializada(false), simulacaoFinalizada(false) {
        // Inicializar os componentes principais
        escalonador = std::make_unique<Escalonador>();
        redeArmazens = std::make_unique<RedeArmazens>();
        sistemaTransporte = std::make_unique<SistemaTransporte>(redeArmazens->obterGrafo());
        // O gerenciador de estatísticas será inicializado após carregar os pacotes
        // e configurar os outros componentes.
    }

    Simulador::~Simulador() {
        limparRecursos();
    }

    void Simulador::carregarParametros(const std::string& arquivoConfig) {
        // Esta função exigiria um parser de arquivo de configuração customizado.
        // Por simplicidade, assumimos que os parâmetros são definidos via linha de comando
        // ou pela sobrecarga ParametrosSimulacao.
        std::cerr << "Carregamento de parametros via arquivo de configuracao nao implementado diretamente aqui." << std::endl;
        std::cerr << "Use a sobrecarga com ParametrosSimulacao ou argumentos de linha de comando." << std::endl;
        throw std::runtime_error("Funcionalidade de carregar parametros de arquivo nao implementada.");
    }

    void Simulador::carregarParametros(const ParametrosSimulacao& params_in) {
        parametros = params_in;
        escalonador->habilitarModoDebug(parametros.modoDebug);
        std::cout << "Parametros da simulacao carregados." << std::endl;
        if (parametros.modoDebug) {
            std::cout << "Modo DEBUG ativado." << std::endl;
            std::cout << "Arquivo Topologia: " << parametros.arquivoTopologia << std::endl;
            std::cout << "Arquivo Pacotes: " << parametros.arquivoPacotes << std::endl;
            std::cout << "Arquivo Saida: " << parametros.arquivoSaida << std::endl;
            std::cout << "Tempo Final: " << parametros.tempoFinal << std::endl;
        }
    }

    void Simulador::definirArquivoTopologia(const std::string& arquivo) {
        parametros.arquivoTopologia = arquivo;
    }

    void Simulador::definirArquivoPacotes(const std::string& arquivo) {
        parametros.arquivoPacotes = arquivo;
    }

    void Simulador::definirArquivoSaida(const std::string& arquivo) {
        parametros.arquivoSaida = arquivo;
    }

    bool Simulador::inicializar() {
        if (simulacaoInicializada) {
            std::cout << "Simulacao ja inicializada. Reinicializando..." << std::endl;
            reinicializar();
        }

        if (!validarParametros() || !validarArquivos()) {
            std::cerr << "Falha na validacao de parametros ou arquivos." << std::endl;
            return false;
        }

        if (!carregarTopologia()) {
            std::cerr << "Falha ao carregar topologia." << std::endl;
            return false;
        }
        if (!carregarPacotes()) {
            std::cerr << "Falha ao carregar pacotes." << std::endl;
            return false;
        }

        configurarSistemas();
        agendarEventosIniciais();

        // Inicializar o gerenciador de estatísticas após todos os componentes estarem prontos
        gerenciadorEstatisticas = std::make_unique<GerenciadorEstatisticas>(
            std::shared_ptr<Escalonador>(escalonador.get(), [](Escalonador*){}), // Shared_ptr para raw pointer sem delete
            std::shared_ptr<RedeArmazens>(redeArmazens.get(), [](RedeArmazens*){}),
            std::shared_ptr<SistemaTransporte>(sistemaTransporte.get(), [](SistemaTransporte*){}),
            pacotes
        );

        if (!validarConsistencia()) {
            std::cerr << "Falha na validacao de consistencia." << std::endl;
            return false;
        }

        simulacaoInicializada = true;
        simulacaoFinalizada = false;
        std::cout << "Simulacao inicializada com sucesso." << std::endl;
        return true;
    }

    void Simulador::reinicializar() {
        escalonador->reiniciarSimulacao();
        redeArmazens->limpar();
        sistemaTransporte->limpar(); // Assumindo um método limpar em SistemaTransporte
        pacotes.clear();
        gerenciadorEstatisticas.reset(); // Reinicia o gerenciador de estatísticas
        simulacaoInicializada = false;
        simulacaoFinalizada = false;
    }

    bool Simulador::executarSimulacao() {
        if (!simulacaoInicializada) {
            std::cerr << "Erro: Simulacao nao inicializada." << std::endl;
            return false;
        }

        std::cout << "Iniciando execucao da simulacao..." << std::endl;
        while (escalonador->temEventosPendentes() &&
               escalonador->obterTempoAtual() < parametros.tempoFinal &&
               !simulacaoFinalizada) {
            escalonador->processarProximoEvento();
            if (escalonador->obterTempoAtual() >= parametros.tempoFinal) {
                std::cout << "Tempo final (" << parametros.tempoFinal << ") atingido." << std::endl;
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
            escalonador->obterTempoAtual() < parametros.tempoFinal &&
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
        // O Simulador age como um IProcessadorEvento, mas sua função principal é orquestrar
        // a chamada dos processadores específicos (e.g., EventoChegada processa no Armazem).
        // Aqui, o simulador delega a responsabilidade.
        if (parametros.modoDebug) {
            std::cout << "[Simulador] Processando evento: " << evento->obterDetalhes() << std::endl;
        }

        // Antes de executar, registrar o processador no escalonador.
        // Isso geralmente é feito na inicialização.
        // Aqui, garantimos que o evento tenha suas dependências configuradas.
        if (evento->obterTipo() == TipoEvento::CHEGADA_PACOTE) {
            auto chegada_evento = std::static_pointer_cast<EventoChegada>(evento);
            auto armazem_id = chegada_evento->obterArmazemDestino();
            chegada_evento->definirArmazem(redeArmazens->obterArmazem(armazem_id));
            chegada_evento->definirEscalonador(std::shared_ptr<Escalonador>(escalonador.get(), [](Escalonador*){})); // Raw pointer para shared_ptr
        } else if (evento->obterTipo() == TipoEvento::TRANSPORTE) {
            auto transporte_evento = std::static_pointer_cast<EventoTransporte>(evento);
            auto origem_id = transporte_evento->obterArmazemOrigem();
            auto destino_id = transporte_evento->obterArmazemDestino();
            transporte_evento->definirArmazemOrigem(redeArmazens->obterArmazem(origem_id));
            transporte_evento->definirArmazemDestino(redeArmazens->obterArmazem(destino_id));
            transporte_evento->definirSistemaTransporte(std::shared_ptr<SistemaTransporte>(sistemaTransporte.get(), [](SistemaTransporte*){}));
            transporte_evento->definirEscalonador(std::shared_ptr<Escalonador>(escalonador.get(), [](Escalonador*){}));
        }

        evento->executar();
    }

    bool Simulador::podeProcessar(TipoEvento tipo) const {
        // O simulador pode "processar" todos os tipos de evento, pois ele os delega.
        // No entanto, ele não os processa diretamente, apenas os encaminha.
        // Se houver lógica específica para o simulador em si (e não para suas entidades),
        // isso seria adicionado aqui.
        return true;
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
            gerenciadorEstatisticas->gerarRelatorioTexto(parametros.arquivoSaida);
        } else {
            std::cerr << "Gerenciador de estatisticas nao inicializado." << std::endl;
        }
    }

    void Simulador::salvarEstatisticas() const {
        salvarEstatisticas(parametros.arquivoSaida + ".bin"); // Exemplo de nome de arquivo binário
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

    bool Simulador::carregarTopologia() {
        ConfiguracaoSistema config_para_leitor;
        config_para_leitor.intervaloTransporte = parametros.intervaloTransporte;
        config_para_leitor.tempoManipulacaoUnitario = parametros.tempoManipulacaoPadrao;
        config_para_leitor.tempoTransportePadrao = parametros.tempoTransportePadrao;
        config_para_leitor.capacidadeTransportePadrao = parametros.capacidadeTransportePadrao;
        config_para_leitor.thresholdGargalo = parametros.thresholdGargalo;

        return LeitorArquivos::lerTopologia(parametros.arquivoTopologia,
                                           std::shared_ptr<RedeArmazens>(redeArmazens.get(), [](RedeArmazens*){}),
                                           std::shared_ptr<SistemaTransporte>(sistemaTransporte.get(), [](SistemaTransporte*){}),
                                           config_para_leitor);
    }

    bool Simulador::carregarPacotes() {
        pacotes = LeitorArquivos::lerPacotes(parametros.arquivoPacotes);
        return !pacotes.empty(); // Sucesso se alguns pacotes foram lidos
    }

    void Simulador::configurarSistemas() {
        // O `LeitorArquivos::lerTopologia` já configura o sistema de transporte.
        // Aqui podemos registrar o Simulador como processador de eventos no Escalonador.
        escalonador->registrarProcessador(TipoEvento::CHEGADA_PACOTE,
                                         std::shared_ptr<IProcessadorEvento>(this, [](IProcessadorEvento*){}));
        escalonador->registrarProcessador(TipoEvento::TRANSPORTE,
                                         std::shared_ptr<IProcessadorEvento>(this, [](IProcessadorEvento*){}));
        // Outros tipos de evento podem ser registrados aqui.
    }

    void Simulador::agendarEventosIniciais() {
        // Agendar eventos de chegada iniciais para todos os pacotes
        for (const auto& pacote : pacotes) {
            auto eventoChegada = std::make_shared<EventoChegada>(pacote, pacote->obterArmazemOrigem(), pacote->obterDataPostagem());
            eventoChegada->definirArmazem(redeArmazens->obterArmazem(pacote->obterArmazemOrigem()));
            eventoChegada->definirEscalonador(std::shared_ptr<Escalonador>(escalonador.get(), [](Escalonador*){}));
            escalonador->agendarEvento(eventoChegada);
        }

        // Agendar os primeiros eventos de transporte para todas as rotas
        sistemaTransporte->agendarTransportesIniciais(0.0); // Começa a agendar a partir do tempo 0
        for (const auto& rota_pair : sistemaTransporte->obterTodasRotas()) {
            auto rota = sistemaTransporte->obterRota(rota_pair.first, rota_pair.second);
            if (rota) {
                auto eventoTransporte = std::make_shared<EventoTransporte>(rota->obterOrigem(), rota->obterDestino(), rota->obterProximoTransporte());
                eventoTransporte->definirArmazemOrigem(redeArmazens->obterArmazem(rota->obterOrigem()));
                eventoTransporte->definirArmazemDestino(redeArmazens->obterArmazem(rota->obterDestino()));
                eventoTransporte->definirSistemaTransporte(std::shared_ptr<SistemaTransporte>(sistemaTransporte.get(), [](SistemaTransporte*){}));
                eventoTransporte->definirEscalonador(std::shared_ptr<Escalonador>(escalonador.get(), [](Escalonador*){}));
                escalonador->agendarEvento(eventoTransporte);
            }
        }
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
        // Os unique_ptr cuidarão da memória
        escalonador.reset();
        redeArmazens.reset();
        sistemaTransporte.reset();
        gerenciadorEstatisticas.reset();
        pacotes.clear();
        std::cout << "Recursos do simulador limpos." << std::endl;
    }

    bool Simulador::validarParametros() const {
        if (parametros.tempoFinal <= 0) {
            std::cerr << "Erro de Parametro: tempoFinal deve ser positivo." << std::endl;
            return false;
        }
        if (parametros.tempoTransportePadrao <= 0 || parametros.tempoManipulacaoPadrao <= 0) {
             std::cerr << "Erro de Parametro: tempos de transporte/manipulacao padrao devem ser positivos." << std::endl;
             return false;
        }
        return true;
    }

    bool Simulador::validarArquivos() const {
        std::ifstream topologia_file(parametros.arquivoTopologia);
        if (!topologia_file.good()) {
            std::cerr << "Erro: Arquivo de topologia nao encontrado ou inacessivel: " << parametros.arquivoTopologia << std::endl;
            return false;
        }
        std::ifstream pacotes_file(parametros.arquivoPacotes);
        if (!pacotes_file.good()) {
            std::cerr << "Erro: Arquivo de pacotes nao encontrado ou inacessivel: " << parametros.arquivoPacotes << std::endl;
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