#include "entidades/Armazem.hpp"
#include <algorithm> // Para std::min, std::remove_if, std::reverse
#include <numeric>   // Para std::accumulate
#include <vector>    // Incluído explicitamente para std::vector, se já não estivesse

namespace LogisticSystem {

    // Implementação da classe Secao
    Secao::Secao(ID_t destino, Capacity_t capacidade, Distance_t tempoManipulacao)
        : armazemDestino(destino), pacotes(capacidade), tempoManipulacaoUnitario(tempoManipulacao),
          ocupacaoMaxima(capacidade) {
        estatisticas = EstatisticasSecao();
    }

    bool Secao::armazenarPacote(std::shared_ptr<Pacote> pacote, Timestamp_t timestamp) {
        if (pacotes.cheia()) {
            return false;
        }
        try {
            pacotes.push(pacote);
            pacote->atualizarEstado(EstadoPacote::ARMAZENADO, timestamp, armazemDestino); // O ID da seção é o destino
            estatisticas.totalPacotesProcessados++; // Considera processado quando armazenado
            // A taxa de ocupação e tempo de permanência serão atualizados nas estatísticas do Armazem
            return true;
        } catch (const std::overflow_error& e) {
            // Pilha cheia, já verificado, mas para robustez
            return false;
        }
    }

    std::shared_ptr<Pacote> Secao::recuperarPacote(size_t posicao, Timestamp_t timestamp) {
        if (posicao >= pacotes.tamanho()) {
            throw std::out_of_range("Posicao invalida para recuperar pacote na secao.");
        }
        // Em uma pilha, remover do meio é custoso.
        // Simulamos o processo de remover os pacotes acima, pegar o desejado
        // e recolocar os demais.
        std::vector<std::shared_ptr<Pacote>> removidos_temporariamente = pacotes.removerAteElemento(posicao);
        std::shared_ptr<Pacote> pacote_recuperado = removidos_temporariamente.back();
        removidos_temporariamente.pop_back(); // Remove o pacote recuperado da lista temporária

        pacotes.recolocarElementos(removidos_temporariamente); // Recoloca os outros pacotes

        // Atualiza o estado do pacote recuperado
        pacote_recuperado->atualizarEstado(EstadoPacote::ALOCADO_TRANSPORTE, timestamp, armazemDestino, "Pacote alocado para transporte.");

        return pacote_recuperado;
    }

    std::vector<std::shared_ptr<Pacote>> Secao::selecionarPacotesMaisAntigos(
        Capacity_t quantidade, Timestamp_t timestamp) {
        std::vector<std::shared_ptr<Pacote>> pacotes_selecionados;

        if (quantidade <= 0 || pacotes.vazia()) {
            return pacotes_selecionados;
        }

        // Determina quantos pacotes podem ser realmente selecionados (min entre quantidade pedida e pacotes disponíveis)
        size_t num_a_selecionar = std::min((size_t)quantidade, pacotes.tamanho());
        if (num_a_selecionar == 0) {
            return pacotes_selecionados;
        }

        // Para pegar os 'num_a_selecionar' pacotes MAIS ANTIGOS de uma Pilha (LIFO),
        // eles estão nas posições mais profundas (próximas da base).
        // A posição do pacote mais antigo da pilha é (tamanho - 1).
        // Se queremos 'N' pacotes mais antigos, o pacote mais "raso" deles estará
        // na posição (tamanho - N).
        // Precisamos remover do topo até a posição do pacote mais "raso" dos que serão selecionados.

        // Posição do pacote mais "raso" entre os 'num_a_selecionar' mais antigos.
        // Se a pilha tem 5 elementos (0 a 4) e queremos 3 (posições 2, 3, 4 - os mais antigos).
        // A posição do mais raso é 2. (tamanho - num_a_selecionar) = 5 - 3 = 2.
        size_t posicao_do_pacote_mais_raso_a_selecionar = pacotes.tamanho() - num_a_selecionar;

        // Remover todos os pacotes do topo até (e incluindo) o pacote na
        // `posicao_do_pacote_mais_raso_a_selecionar`.
        // A função `removerAteElemento` desempilha a partir do topo até o elemento desejado.
        // O vetor `removidos_temporariamente` conterá os pacotes do topo (mais recente)
        // até o pacote na `posicao_do_pacote_mais_raso_a_selecionar` (mais antigo).
        // Ex: Pilha [P5, P4, P3, P2, P1]. Se `num_a_selecionar` = 3, `posicao` = 2 (P3).
        // `removerAteElemento(2)` retorna [P5, P4, P3]. A pilha fica vazia.
        std::vector<std::shared_ptr<Pacote>> removidos_temporariamente =
            pacotes.removerAteElemento(posicao_do_pacote_mais_raso_a_selecionar);

        // O vetor `removidos_temporariamente` agora está na ordem do mais recente para o mais antigo.
        // Os 'num_a_selecionar' pacotes mais antigos estão no *final* deste vetor.
        // Precisamos extraí-los do final e adicioná-los a `pacotes_selecionados`.

        // Selecionar os 'num_a_selecionar' pacotes do final (os mais antigos)
        for (size_t i = 0; i < num_a_selecionar; ++i) {
            // Pega o último elemento (que é o mais antigo entre os removidos e ainda não selecionados)
            std::shared_ptr<Pacote> pacote_selecionado = removidos_temporariamente.back();
            removidos_temporariamente.pop_back(); // Remove-o do vetor temporário

            pacotes_selecionados.push_back(pacote_selecionado);

            // Atualiza estado do pacote para alocado para transporte
            pacote_selecionado->atualizarEstado(EstadoPacote::ALOCADO_TRANSPORTE, timestamp, armazemDestino, "Pacote selecionado para transporte.");
        }

        // Os pacotes restantes em `removidos_temporariamente` (se houver) foram desempilhados,
        // mas não foram selecionados para transporte. Eles precisam ser recolocados na pilha.
        // Eles estão na ordem do mais recente (topo) para o mais antigo (base).
        // Para recolocar na pilha, `recolocarElementos` espera que o vetor esteja na ordem
        // do mais antigo para o mais recente (base para topo da pilha).
        // Então, é necessário inverter a ordem dos elementos restantes antes de recolocá-los.
        std::reverse(removidos_temporariamente.begin(), removidos_temporariamente.end());
        pacotes.recolocarElementos(removidos_temporariamente);

        return pacotes_selecionados;
    }

    Distance_t Secao::calcularTempoManipulacao(size_t posicao) const {
        return pacotes.calcularTempoAcesso(posicao, tempoManipulacaoUnitario);
    }

    std::shared_ptr<Pacote> Secao::obterPacoteMaisAntigo() const {
        if (vazia()) {
            return nullptr;
        }
        // Em uma pilha LIFO, o pacote mais antigo é o que está na base (última posição).
        // A Pilha::obterElemento(posicao) acessa o elemento sem removê-lo.
        return pacotes.obterElemento(pacotes.tamanho() - 1); // Retorna o pacote na base da pilha (o mais antigo)
    }

    void Secao::atualizarEstatisticas(Timestamp_t timestampAtual) {
        // Atualiza a taxa de ocupação média
        double ocupacao_atual = static_cast<double>(pacotes.tamanho());
        // A lógica para `taxaOcupacaoMedia` parece ser uma média ponderada sobre
        // `totalPacotesProcessados`. Se `totalPacotesProcessados` é o número de pacotes
        // que já passaram pela seção, essa média está correta.
        estatisticas.taxaOcupacaoMedia =
            (estatisticas.taxaOcupacaoMedia * estatisticas.totalPacotesProcessados + ocupacao_atual) / (estatisticas.totalPacotesProcessados + 1);
        estatisticas.capacidadeMaximaUtilizada = std::max(estatisticas.capacidadeMaximaUtilizada, (int)ocupacao_atual);

        // O tempo médio de permanência e tempo máximo de manipulação
        // são mais complexos de calcular sem um registro detalhado de entrada/saída de cada pacote.
        // Eles seriam idealmente calculados no momento em que um pacote é removido.
        // Por agora, podem permanecer com 0.0 ou ser calculados de forma simplificada
        // apenas para pacotes que foram completamente processados (entrada e saída).
    }


    // Implementação da classe Armazem
    Armazem::Armazem(ID_t identificador, const std::string& nomeArmazem,
            Capacity_t capacidade)
        : id(identificador), nome(nomeArmazem), capacidadeTotal(capacidade),
          pacotesAtivos(0), taxaOcupacao(0.0) {}

    void Armazem::adicionarSecao(ID_t armazemDestino, Capacity_t capacidadeSecao,
                               Distance_t tempoManipulacao) {
        if (secoes.count(armazemDestino)) {
            // Secao já existe, pode ser um erro ou uma atualização de capacidade
            return;
        }
        secoes[armazemDestino] = std::make_unique<Secao>(armazemDestino, capacidadeSecao, tempoManipulacao);
    }

    void Armazem::removerSecao(ID_t armazemDestino) {
        secoes.erase(armazemDestino);
    }

    bool Armazem::receberPacote(std::shared_ptr<Pacote> pacote, Timestamp_t timestamp) {
        ID_t proximo_armazem_na_rota = pacote->obterProximoArmazem();

        // Se o pacote chegou ao destino final do armazém
        if (pacote->chegouDestino() && pacote->obterArmazemDestino() == id) {
            pacote->atualizarEstado(EstadoPacote::ENTREGUE, timestamp, id, "Pacote entregue ao destino final.");
            // Não armazenamos pacotes entregues em seções.
            // Precisamos decrementar pacotesAtivos.
            pacotesAtivos--;
            notificarObservadores(pacote); // Notificar sobre pacote entregue
            return true;
        }

        // Tenta encontrar a seção correspondente ao próximo destino na rota do pacote
        if (secoes.count(proximo_armazem_na_rota)) {
            bool armazenado = secoes[proximo_armazem_na_rota]->armazenarPacote(pacote, timestamp);
            if (armazenado) {
                pacotesAtivos++;
                notificarObservadores(pacote); // Notificar sobre pacote armazenado
                return true;
            }
        }
        // Se não encontrou seção ou seção cheia para o destino específico
        // Tentar armazenar em uma seção genérica ou tratar como erro.
        // Para este protótipo, se não há seção específica, falha.
        pacote->atualizarEstado(EstadoPacote::CHEGOU_NAO_ARMAZENADO, timestamp, id, "Chegou mas nao pode ser armazenado.");
        return false;
    }

    std::vector<std::shared_ptr<Pacote>> Armazem::prepararTransporte(
        ID_t armazemDestino, Capacity_t capacidadeTransporte, Timestamp_t timestamp) {
        std::vector<std::shared_ptr<Pacote>> pacotes_para_transporte;
        if (secoes.count(armazemDestino)) {
            pacotes_para_transporte = secoes[armazemDestino]->selecionarPacotesMaisAntigos(
                capacidadeTransporte, timestamp);
            // Atualizar contagem de pacotes ativos (eles foram removidos do armazém para o transporte)
            pacotesAtivos -= pacotes_para_transporte.size();
        }
        return pacotes_para_transporte;
    }

    int Armazem::obterPacotesAtivos() const {
        int total = 0;
        for (const auto& pair : secoes) {
            total += pair.second->obterOcupacao();
        }
        return total;
    }

    double Armazem::obterTaxaOcupacao() const {
        if (capacidadeTotal == 0) return 0.0;
        return static_cast<double>(obterPacotesAtivos()) / capacidadeTotal;
    }

    bool Armazem::temSecao(ID_t armazemDestino) const {
        return secoes.count(armazemDestino);
    }

    const Secao* Armazem::obterSecao(ID_t armazemDestino) const {
        if (temSecao(armazemDestino)) {
            return secoes.at(armazemDestino).get();
        }
        return nullptr;
    }

    std::vector<ID_t> Armazem::obterDestinosDisponiveis() const {
        std::vector<ID_t> destinos;
        for (const auto& pair : secoes) {
            destinos.push_back(pair.first);
        }
        return destinos;
    }

    bool Armazem::estaVazio() const {
        return obterPacotesAtivos() == 0;
    }

    bool Armazem::temPacotesParaTransporte(ID_t destino) const {
        if (secoes.count(destino)) {
            return !secoes.at(destino)->vazia();
        }
        return false;
    }

    void Armazem::atualizarEstatisticas(Timestamp_t timestampAtual) {
        // Atualiza o histórico de ocupação
        historicoOcupacao.push_back({timestampAtual, obterTaxaOcupacao()});

        // Atualiza estatísticas de cada seção
        for (auto& pair : secoes) {
            pair.second->atualizarEstatisticas(timestampAtual);
        }
    }

    std::unordered_map<ID_t, EstatisticasSecao> Armazem::obterEstatisticasSecoes() const {
        std::unordered_map<ID_t, EstatisticasSecao> stats;
        for (const auto& pair : secoes) {
            stats[pair.first] = pair.second->obterEstatisticas();
        }
        return stats;
    }

    std::vector<ID_t> Armazem::identificarSecoesSObrecarregadas(double threshold) const {
        std::vector<ID_t> sobrecarregadas;
        for (const auto& pair : secoes) {
            if (static_cast<double>(pair.second->obterOcupacao()) / pair.second->obterCapacidadeMaxima() > threshold) {
                sobrecarregadas.push_back(pair.first);
            }
        }
        return sobrecarregadas;
    }

    Distance_t Armazem::calcularTempoMedioManipulacao() const {
        Distance_t total_tempo_manipulacao = 0.0;
        int total_secoes_com_dados = 0;
        for (const auto& pair : secoes) {
            // Simplificado: usa o tempo unitário da seção, sem considerar a profundidade
            if (pair.second->obterOcupacao() > 0) {
                 total_tempo_manipulacao += pair.second->calcularTempoManipulacao(0); // Usando a posição 0 para o cálculo base
                 total_secoes_com_dados++;
            }
        }
        return total_secoes_com_dados > 0 ? total_tempo_manipulacao / total_secoes_com_dados : 0.0;
    }

} // namespace LogisticSystem