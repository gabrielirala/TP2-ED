#include "entidades/Armazem.hpp"
#include <algorithm> // Para std::min, std::remove_if, std::reverse
#include <numeric>   // Para std::accumulate
#include <vector>    // Necessário para std::vector

namespace LogisticSystem {

    // Implementação da classe Secao
    Secao::Secao(ID_t destino, Capacity_t capacidade, Distance_t tempoManipulacao)
        : armazemDestino(destino), pacotes(capacidade), tempoManipulacaoUnitario(tempoManipulacao),
          ocupacaoMaxima(capacidade) {
        estatisticas = EstatisticasSecao();
    }

    bool Secao::armazenarPacote(std::shared_ptr<Pacote> pacote, Timestamp_t timestamp) {
        if (pacotes.cheia()) {
            // Observação: se a seção está cheia e o pacote não pode ser armazenado,
            // seu estado deveria ser atualizado para algo como CHEGOU_NAO_ARMAZENADO
            // se esse estado ainda fosse válido ou se houvesse uma lógica de fila de espera.
            // No novo enunciado, CHEGOU_NAO_ARMAZENADO foi removido.
            // A responsabilidade de lidar com pacotes não armazenados pode ser do Simulador.
            return false;
        }
        try {
            pacotes.push(pacote);
            // Estado 3: Armazenado na seção associada ao próximo destino de um armazém
            pacote->atualizarEstado(EstadoPacote::ARMAZENADO, timestamp, armazemDestino, "Pacote armazenado na secao.");
            estatisticas.totalPacotesProcessados++; // Considera processado quando armazenado
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
        // Simulamos o processo de remover os pacotes acima, pegar o desejado e recolocar os demais.
        std::vector<std::shared_ptr<Pacote>> removidos_temporariamente = pacotes.removerAteElemento(posicao);
        std::shared_ptr<Pacote> pacote_recuperado = removidos_temporariamente.back();
        removidos_temporariamente.pop_back(); // Remove o pacote recuperado da lista temporária

        // Recolocar os outros pacotes que foram desempilhados mas não foram selecionados
        // É CRUCIAL que esses pacotes tenham seu estado atualizado para "ARMAZENADO" com observação "rearmazenado"
        // E que sejam recolocados na ordem correta (inverter antes de recolocar)
        std::reverse(removidos_temporariamente.begin(), removidos_temporariamente.end()); // Inverte para recolocar na ordem LIFO
        for (const auto& p : removidos_temporariamente) {
            p->atualizarEstado(EstadoPacote::ARMAZENADO, timestamp, armazemDestino, "Pacote rearmazenado na secao.");
        }
        pacotes.recolocarElementos(removidos_temporariamente);

        // Atualiza o estado do pacote recuperado
        // Estado 4: Removido da seção para transporte
        pacote_recuperado->atualizarEstado(EstadoPacote::REMOVIDO_PARA_TRANSPORTE, timestamp, armazemDestino, "Pacote removido da secao para transporte.");

        return pacote_recuperado;
    }

    std::vector<std::shared_ptr<Pacote>> Secao::selecionarPacotesMaisAntigos(
        Capacity_t quantidade, Timestamp_t timestamp) {
        std::vector<std::shared_ptr<Pacote>> pacotes_selecionados;

        if (quantidade <= 0 || pacotes.vazia()) {
            return pacotes_selecionados;
        }

        size_t num_a_selecionar = std::min((size_t)quantidade, pacotes.tamanho());
        if (num_a_selecionar == 0) {
            return pacotes_selecionados;
        }

        // Posição do pacote mais "raso" entre os 'num_a_selecionar' mais antigos.
        size_t posicao_do_pacote_mais_raso_a_selecionar = pacotes.tamanho() - num_a_selecionar;

        // Remover todos os pacotes do topo até (e incluindo) o pacote na
        // `posicao_do_pacote_mais_raso_a_selecionar`.
        std::vector<std::shared_ptr<Pacote>> removidos_temporariamente =
            pacotes.removerAteElemento(posicao_do_pacote_mais_raso_a_selecionar);

        // O vetor `removidos_temporariamente` agora está na ordem do mais recente para o mais antigo.
        // Os 'num_a_selecionar' pacotes mais antigos estão no *final* deste vetor.

        // Selecionar os 'num_a_selecionar' pacotes do final (os mais antigos)
        for (size_t i = 0; i < num_a_selecionar; ++i) {
            std::shared_ptr<Pacote> pacote_selecionado = removidos_temporariamente.back();
            removidos_temporariamente.pop_back();

            pacotes_selecionados.push_back(pacote_selecionado);

            // Atualiza estado do pacote para o novo estado de remoção
            // Estado 4: Removido da seção para transporte
            pacote_selecionado->atualizarEstado(EstadoPacote::REMOVIDO_PARA_TRANSPORTE, timestamp, armazemDestino, "Pacote removido da secao para transporte.");
        }

        // Os pacotes restantes em `removidos_temporariamente` (se houver) foram desempilhados,
        // mas não foram selecionados para transporte. Eles precisam ser recolocados na pilha.
        // Inverter a ordem para manter a lógica LIFO ao recolocar.
        std::reverse(removidos_temporariamente.begin(), removidos_temporariamente.end());
        
        // Atualizar estado para "ARMAZENADO" com observação "rearmazenado" para os pacotes recolocados
        for (const auto& p : removidos_temporariamente) {
            p->atualizarEstado(EstadoPacote::ARMAZENADO, timestamp, armazemDestino, "Pacote rearmazenado na secao.");
        }

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
        // O pacote mais antigo está na base da pilha (última posição).
        return pacotes.obterElemento(pacotes.tamanho() - 1);
    }

    void Secao::atualizarEstatisticas(Timestamp_t timestampAtual) {
        double ocupacao_atual = static_cast<double>(pacotes.tamanho());
        estatisticas.taxaOcupacaoMedia =
            (estatisticas.taxaOcupacaoMedia * estatisticas.totalPacotesProcessados + ocupacao_atual) / (estatisticas.totalPacotesProcessados + 1);
        estatisticas.capacidadeMaximaUtilizada = std::max(estatisticas.capacidadeMaximaUtilizada, (int)ocupacao_atual);
    }

    // Implementação da classe Armazem
    Armazem::Armazem(ID_t identificador, const std::string& nomeArmazem,
            Capacity_t capacidade)
        : id(identificador), nome(nomeArmazem), capacidadeTotal(capacidade),
          pacotesAtivos(0), taxaOcupacao(0.0) {}

    void Armazem::adicionarSecao(ID_t armazemDestino, Capacity_t capacidadeSecao,
                               Distance_t tempoManipulacao) {
        if (secoes.count(armazemDestino)) {
            return;
        }
        secoes[armazemDestino] = std::make_unique<Secao>(armazemDestino, capacidadeSecao, tempoManipulacao);
    }

    void Armazem::removerSecao(ID_t armazemDestino) {
        secoes.erase(armazemDestino);
    }

    bool Armazem::receberPacote(std::shared_ptr<Pacote> pacote, Timestamp_t timestamp) {
        ID_t proximo_armazem_na_rota = pacote->obterProximoArmazem();

        // Se o pacote chegou ao destino final do armazém atual (ou seja, o armazém atual é o destino final)
        if (pacote->chegouDestino() && pacote->obterArmazemDestino() == id) {
            // Estado 5: Entregue
            pacote->atualizarEstado(EstadoPacote::ENTREGUE, timestamp, id, "Pacote entregue ao destino final.");
            pacotesAtivos--; // Decrementa pois o pacote não está mais no sistema (se ele foi contado como ativo)
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
        // O enunciado v1.1 não tem mais o estado CHEGOU_NAO_ARMAZENADO,
        // então pacotes não armazenados devem ser tratados como uma falha ou outra lógica.
        // Por enquanto, apenas retorna false.
        return false;
    }

    std::vector<std::shared_ptr<Pacote>> Armazem::prepararTransporte(
        ID_t armazemDestino, Capacity_t capacidadeTransporte, Timestamp_t timestamp) {
        std::vector<std::shared_ptr<Pacote>> pacotes_para_transporte;
        if (secoes.count(armazemDestino)) {
            pacotes_para_transporte = secoes[armazemDestino]->selecionarPacotesMaisAntigos(
                capacidadeTransporte, timestamp);
            // Atualizar contagem de pacotes ativos (eles foram removidos do armazém para o transporte)
            // Pacotes selecionados para transporte não estão mais na pilha, mas ainda são 'ativos' no sistema.
            // A contagem `pacotesAtivos` aqui pode precisar de revisão dependendo de como
            // `pacotesAtivos` é definido (no armazém vs. no sistema de transporte).
            // Por simplicidade, assumimos que eles saíram do armazém, então removemos da contagem local.
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
        historicoOcupacao.push_back({timestampAtual, obterTaxaOcupacao()});
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
            if (pair.second->obterOcupacao() > 0) {
                 // Usando a posição 0 para o cálculo base para simular o tempo de manipulação do topo
                 // ou da posição mais "rasa" que seria removida rapidamente.
                 // Se o `custoRemocaoGlobal` é o custo por pacote, então esta soma está correta.
                 total_tempo_manipulacao += pair.second->calcularTempoManipulacao(0); 
                 total_secoes_com_dados++;
            }
        }
        return total_secoes_com_dados > 0 ? total_tempo_manipulacao / total_secoes_com_dados : 0.0;
    }

} // namespace LogisticSystem
