#include "entidades/SistemaTransporte.hpp"
#include <stdexcept> // Para std::runtime_error
#include <limits>    // Para std::numeric_limits

namespace LogisticSystem {

    // Implementação da classe RotaTransporte
    RotaTransporte::RotaTransporte(ID_t origem, ID_t destino, Distance_t tempo, Capacity_t capacidade)
        : armazemOrigem(origem), armazemDestino(destino), tempoTransporte(tempo),
          capacidadeMaxima(capacidade), proximoTransporte(0.0) {
        estatisticas = EstatisticasRota();
    }

    void RotaTransporte::agendarProximoTransporte(Timestamp_t timestamp) {
        proximoTransporte = timestamp;
    }

    bool RotaTransporte::podeTransportar(Timestamp_t timestampAtual) const {
        return timestampAtual >= proximoTransporte;
    }

    void RotaTransporte::registrarViagem(Capacity_t pacotesTransportados, Distance_t tempoReal) {
        estatisticas.viagensRealizadas++;
        estatisticas.totalPacotesTransportados += pacotesTransportados;

        // Atualiza a taxa de utilização média
        double utilizacao_atual = static_cast<double>(pacotesTransportados) / capacidadeMaxima;
        estatisticas.taxaUtilizacaoMedia =
            (estatisticas.taxaUtilizacaoMedia * (estatisticas.viagensRealizadas - 1) + utilizacao_atual) / estatisticas.viagensRealizadas;

        // Atualiza capacidade média utilizada
        estatisticas.capacidadeMediaUtilizada =
            (estatisticas.capacidadeMediaUtilizada * (estatisticas.viagensRealizadas - 1) + pacotesTransportados) / estatisticas.viagensRealizadas;

        // Atualiza tempo médio de viagem
        estatisticas.tempoMedioViagem =
            (estatisticas.tempoMedioViagem * (estatisticas.viagensRealizadas - 1) + tempoReal) / estatisticas.viagensRealizadas;
    }

    // Implementação da classe SistemaTransporte
    SistemaTransporte::SistemaTransporte(std::shared_ptr<Grafo> rede)
        : redeArmazens(rede), intervaloTransporte(0.0), tempoTransportePadrao(0.0), capacidadePadrao(0) {}

    std::string SistemaTransporte::criarChaveRota(ID_t origem, ID_t destino) const {
        return std::to_string(origem) + "-" + std::to_string(destino);
    }

    void SistemaTransporte::configurarParametrosGlobais(Distance_t intervalo, Distance_t tempo,
                                                       Capacity_t capacidade) {
        intervaloTransporte = intervalo;
        tempoTransportePadrao = tempo;
        capacidadePadrao = capacidade;
    }

    void SistemaTransporte::adicionarRota(ID_t origem, ID_t destino, Distance_t tempo, Capacity_t capacidade) {
        if (!redeArmazens->existeVertice(origem) || !redeArmazens->existeVertice(destino)) {
            throw std::runtime_error("Nao eh possivel adicionar rota: origem ou destino nao existem na rede de armazens.");
        }
        std::string chave = criarChaveRota(origem, destino);
        rotas[chave] = std::make_unique<RotaTransporte>(origem, destino, tempo, capacidade);
        // Garante que o grafo reflita a aresta com peso e capacidade
        if (!redeArmazens->existeAresta(origem, destino)) {
             redeArmazens->adicionarAresta(origem, destino, tempo, capacidade);
        }
    }

    void SistemaTransporte::removerRota(ID_t origem, ID_t destino) {
        std::string chave = criarChaveRota(origem, destino);
        rotas.erase(chave);
        // Também remove a aresta do grafo, se existir
        if (redeArmazens->existeAresta(origem, destino)) {
            redeArmazens->removerAresta(origem, destino);
        }
    }

    bool SistemaTransporte::podeExecutarTransporte(ID_t origem, ID_t destino, Timestamp_t timestamp) const {
        std::string chave = criarChaveRota(origem, destino);
        if (rotas.count(chave)) {
            return rotas.at(chave)->podeTransportar(timestamp);
        }
        return false;
    }

    Timestamp_t SistemaTransporte::calcularTempoChegada(ID_t origem, ID_t destino, Timestamp_t saida) const {
        std::string chave = criarChaveRota(origem, destino);
        if (rotas.count(chave)) {
            return saida + rotas.at(chave)->obterTempoTransporte();
        }
        // Se a rota não existe, pode-se usar um tempo padrão ou lançar erro
        return saida + tempoTransportePadrao; // Usar tempo padrão como fallback
    }

    void SistemaTransporte::registrarTransporteExecutado(ID_t origem, ID_t destino,
                                                        Capacity_t pacotesTransportados, Timestamp_t timestamp) {
        std::string chave = criarChaveRota(origem, destino);
        if (rotas.count(chave)) {
            // O tempo real de viagem poderia ser passado para cá, mas para simplificar, usamos o tempo da rota.
            rotas[chave]->registrarViagem(pacotesTransportados, rotas[chave]->obterTempoTransporte());
            // Agendar o próximo transporte para esta rota
            agendarProximoTransporte(origem, destino, timestamp);
        }
    }

    void SistemaTransporte::agendarTransportesIniciais(Timestamp_t timestampInicial) {
        for (auto& pair : rotas) {
            pair.second->agendarProximoTransporte(timestampInicial + intervaloTransporte); // O primeiro agendamento
        }
    }

    void SistemaTransporte::agendarProximoTransporte(ID_t origem, ID_t destino, Timestamp_t timestampAtual) {
        std::string chave = criarChaveRota(origem, destino);
        if (rotas.count(chave)) {
            // O próximo transporte será agendado a partir do timestamp atual + o intervalo padrão
            rotas[chave]->agendarProximoTransporte(timestampAtual + intervaloTransporte);
        }
    }

    const RotaTransporte* SistemaTransporte::obterRota(ID_t origem, ID_t destino) const {
        std::string chave = criarChaveRota(origem, destino);
        if (rotas.count(chave)) {
            return rotas.at(chave).get();
        }
        return nullptr;
    }

    std::vector<std::pair<ID_t, ID_t>> SistemaTransporte::obterTodasRotas() const {
        std::vector<std::pair<ID_t, ID_t>> todas_rotas;
        for (const auto& pair : rotas) {
            todas_rotas.push_back({pair.second->obterOrigem(), pair.second->obterDestino()});
        }
        return todas_rotas;
    }

    bool SistemaTransporte::existeRota(ID_t origem, ID_t destino) const {
        return rotas.count(criarChaveRota(origem, destino));
    }

    std::unordered_map<std::string, EstatisticasRota> SistemaTransporte::obterEstatisticasTodasRotas() const {
        std::unordered_map<std::string, EstatisticasRota> stats;
        for (const auto& pair : rotas) {
            stats[pair.first] = pair.second->obterEstatisticas();
        }
        return stats;
    }

    double SistemaTransporte::calcularEficienciaGeral() const {
        double total_taxa_utilizacao = 0.0;
        int rotas_com_viagens = 0;
        for (const auto& pair : rotas) {
            if (pair.second->obterEstatisticas().viagensRealizadas > 0) {
                total_taxa_utilizacao += pair.second->obterEstatisticas().taxaUtilizacaoMedia;
                rotas_com_viagens++;
            }
        }
        return rotas_com_viagens > 0 ? total_taxa_utilizacao / rotas_com_viagens : 0.0;
    }

    std::vector<std::pair<ID_t, ID_t>> SistemaTransporte::identificarRotasSobrecarregadas(double threshold) const {
        std::vector<std::pair<ID_t, ID_t>> rotas_sobrecarregadas;
        for (const auto& pair : rotas) {
            if (pair.second->obterEstatisticas().taxaUtilizacaoMedia > threshold) {
                rotas_sobrecarregadas.push_back({pair.second->obterOrigem(), pair.second->obterDestino()});
            }
        }
        return rotas_sobrecarregadas;
    }

    void SistemaTransporte::otimizarCapacidades(const std::unordered_map<std::string, double>& demandas) {
        // Lógica de otimização de capacidades (ex: ajustar capacidade com base na demanda)
        // Isso seria um algoritmo mais complexo, possivelmente iterativo.
        // Para este protótipo, será uma implementação básica.
        for (auto& pair : rotas) {
            std::string chave = criarChaveRota(pair.second->obterOrigem(), pair.second->obterDestino());
            if (demandas.count(chave)) {
                // Aumenta a capacidade se a demanda for significativamente maior que a capacidade atual
                // Ou diminui se a demanda for muito baixa
                Capacity_t nova_capacidade = static_cast<Capacity_t>(demandas.at(chave) * 1.2); // Exemplo: 20% acima da demanda
                if (nova_capacidade > pair.second->obterCapacidadeMaxima()) {
                    pair.second->definirCapacidadeMaxima(nova_capacidade);
                }
                // Poderia também diminuir, mas precisa de regras para não estrangular a rota
            }
        }
    }

    void SistemaTransporte::balancearFrequencias() {
        // Lógica para balancear a frequência de transporte (intervaloTransporte)
        // Isso também seria um algoritmo mais complexo, possivelmente ajustando
        // `intervaloTransporte` para rotas específicas.
        // Para este protótipo, não há implementacao específica.
    }

} // namespace LogisticSystem