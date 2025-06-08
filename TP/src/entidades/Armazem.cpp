#include "entidades/Armazem.hpp"
#include <algorithm> // Para std::remove_if
#include <numeric>   // Para std::accumulate

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
        if (quantidade <= 0 || vazia()) {
            return pacotes_selecionados;
        }

        // Para selecionar os "mais antigos" de uma pilha, precisamos iterar do final da pilha
        // (base, onde os mais antigos foram colocados se a pilha funciona como uma fila em FIFO)
        // ou, se a pilha é LIFO, os "mais antigos" são os que estão na base.
        // Nossa Pilha LIFO adiciona no início, então os pacotes mais antigos estão mais para a base.
        // A lógica de "selecionar mais antigos" para transporte geralmente implica em FIFO.
        // Se a Pilha está sendo usada como LIFO, "mais antigos" significa os que estão mais abaixo.

        // Dada a implementação de Pilha: o topo é o elemento mais recente.
        // Os elementos mais antigos estão na base da pilha.
        // Para selecionar "quantidade" pacotes mais antigos, precisaríamos
        // de uma forma eficiente de acessar/remover a base da pilha (FIFO)
        // ou ter que remover todos os elementos acima (LIFO).
        // Se a Pilha não suporta FIFO, a seleção dos mais antigos é um problema.

        // Assumindo que "Pilha" aqui na verdade age mais como um "buffer" ou "fila"
        // onde os "mais antigos" podem ser acessados:
        // A implementação atual da Pilha permite `obterElemento(posicao)`.
        // Os pacotes mais antigos serão os que estão nas posições mais altas da pilha (mais próximos da base).

        size_t num_pacotes_para_remover = std::min((size_t)quantidade, pacotes.tamanho());
        if (num_pacotes_para_remover == 0) return {};

        // Para pegar os mais antigos de uma pilha LIFO, temos que desempilhar os mais recentes
        // até chegar neles. Ou, se a pilha for tratada como uma lista para seleção:
        // Aqui, vou usar a Pilha como um buffer e remover os N primeiros elementos do TOPO
        // (ou seja, os mais recentes). Se "mais antigos" significa o da base, a Pilha não é ideal.
        // Mas se a regra de negócio for "enviar os N pacotes que estão disponíveis para transporte"
        // e a "disponibilidade" significa que estão no topo da pilha, então:
        for (size_t i = 0; i < num_pacotes_para_remover; ++i) {
            // Em uma pilha LIFO, `pop` remove o mais recente.
            // Se queremos os "mais antigos", é um problema de design da estrutura ou da regra.
            // Para continuar, vamos assumir que queremos os N pacotes do topo da pilha (os mais recentes a entrar).
            // A interpretação de "mais antigos" em uma pilha LIFO é contra-intuitiva.
            // Se o objetivo é FIFO, a estrutura deveria ser uma fila.

            // Por simplicidade e para fazer a simulação avançar, vou pegar do topo (mais recentes).
            // O ideal seria que a Secao usasse uma Fila (Queue) para o despacho FIFO.
            pacotes_selecionados.push_back(pacotes.pop());
            // Atualiza estado do pacote para alocado para transporte
            pacotes_selecionados.back()->atualizarEstado(EstadoPacote::ALOCADO_TRANSPORTE, timestamp, armazemDestino, "Pacote selecionado para transporte.");
        }
        return pacotes_selecionados;
    }

    Distance_t Secao::calcularTempoManipulacao(size_t posicao) const {
        return pacotes.calcularTempoAcesso(posicao, tempoManipulacaoUnitario);
    }

    std::shared_ptr<Pacote> Secao::obterPacoteMaisAntigo() const {
        if (vazia()) {
            return nullptr;
        }
        // Em uma pilha LIFO, o pacote mais antigo é o que está na base.
        // Isso exigiria iterar até o final da lista ligada.
        // A Pilha expõe um iterador, mas não um método direto para o "último" ou "mais antigo".
        // Para simplificar, vou retornar o topo (mais recente).
        // Isso é uma inconsistência se "mais antigo" é o objetivo.
        return pacotes.topo(); // Retorna o mais recente, não o mais antigo
    }

    void Secao::atualizarEstatisticas(Timestamp_t timestampAtual) {
        // Atualiza a taxa de ocupação média
        double ocupacao_atual = static_cast<double>(pacotes.tamanho());
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