#ifndef SISTEMA_TRANSPORTE_HPP
#define SISTEMA_TRANSPORTE_HPP

#include "utils/Tipos.hpp"
#include "estruturas/Grafo.hpp"
#include "interfaces/IObservador.hpp"
#include <memory>
#include <unordered_map>

namespace LogisticSystem {
    struct EstatisticasRota {
        int viagensRealizadas;
        double taxaUtilizacaoMedia;
        Capacity_t capacidadeMediaUtilizada;
        Distance_t tempoMedioViagem;
        int totalPacotesTransportados;
        
        EstatisticasRota() : viagensRealizadas(0), taxaUtilizacaoMedia(0.0),
                           capacidadeMediaUtilizada(0), tempoMedioViagem(0.0),
                           totalPacotesTransportados(0) {}
    };
    
    class RotaTransporte {
    private:
        ID_t armazemOrigem;
        ID_t armazemDestino;
        Distance_t tempoTransporte;
        Capacity_t capacidadeMaxima;
        Timestamp_t proximoTransporte;
        EstatisticasRota estatisticas;
        
    public:
        RotaTransporte(ID_t origem, ID_t destino, Distance_t tempo, Capacity_t capacidade);
        
        // Getters
        ID_t obterOrigem() const { return armazemOrigem; }
        ID_t obterDestino() const { return armazemDestino; }
        Distance_t obterTempoTransporte() const { return tempoTransporte; }
        Capacity_t obterCapacidadeMaxima() const { return capacidadeMaxima; }
        Timestamp_t obterProximoTransporte() const { return proximoTransporte; }
        
        // Agendamento
        void agendarProximoTransporte(Timestamp_t timestamp);
        bool podeTransportar(Timestamp_t timestampAtual) const;
        
        // Estatísticas
        void registrarViagem(Capacity_t pacotesTransportados, Distance_t tempoReal);
        const EstatisticasRota& obterEstatisticas() const { return estatisticas; }
        
        // Configuração
        void definirCapacidadeMaxima(Capacity_t novaCapacidade) { capacidadeMaxima = novaCapacidade; }
        void definirTempoTransporte(Distance_t novoTempo) { tempoTransporte = novoTempo; }
    };
    
    class SistemaTransporte {
    private:
        std::shared_ptr<Grafo> redeArmazens;
        std::unordered_map<std::string, std::unique_ptr<RotaTransporte>> rotas;
        
        // Configurações globais
        Distance_t intervaloTransporte;
        Distance_t tempoTransportePadrao;
        Capacity_t capacidadePadrao;
        
        std::string criarChaveRota(ID_t origem, ID_t destino) const;
        
    public:
        explicit SistemaTransporte(std::shared_ptr<Grafo> rede);
        
        // Configuração do sistema
        void configurarParametrosGlobais(Distance_t intervalo, Distance_t tempo, 
                                       Capacity_t capacidade);
        void adicionarRota(ID_t origem, ID_t destino, Distance_t tempo, Capacity_t capacidade);
        void removerRota(ID_t origem, ID_t destino);
        
        // Operações de transporte
        bool podeExecutarTransporte(ID_t origem, ID_t destino, Timestamp_t timestamp) const;
        Timestamp_t calcularTempoChegada(ID_t origem, ID_t destino, Timestamp_t saida) const;
        void registrarTransporteExecutado(ID_t origem, ID_t destino, 
                                        Capacity_t pacotesTransportados, Timestamp_t timestamp);
        
        // Agendamento
        void agendarTransportesIniciais(Timestamp_t timestampInicial);
        void agendarProximoTransporte(ID_t origem, ID_t destino, Timestamp_t timestampAtual);
        
        // Consultas
        const RotaTransporte* obterRota(ID_t origem, ID_t destino) const;
        std::vector<std::pair<ID_t, ID_t>> obterTodasRotas() const;
        bool existeRota(ID_t origem, ID_t destino) const;
        
        // Estatísticas
        std::unordered_map<std::string, EstatisticasRota> obterEstatisticasTodasRotas() const;
        double calcularEficienciaGeral() const;
        std::vector<std::pair<ID_t, ID_t>> identificarRotasSobrecarregadas(double threshold) const;
        
        // Otimização
        void otimizarCapacidades(const std::unordered_map<std::string, double>& demandas);
        void balancearFrequencias();
    };
}

#endif
