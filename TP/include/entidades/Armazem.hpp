#ifndef ARMAZEM_HPP
#define ARMAZEM_HPP

#include "Pacote.hpp"
#include "estruturas/Pilha.hpp"
#include "utils/Tipos.hpp"
#include "interfaces/IObservador.hpp"
#include <unordered_map>
#include <memory>
#include <vector>

namespace LogisticSystem {
    struct EstatisticasSecao {
        int totalPacotesProcessados;
        Timestamp_t tempoMedioPermanencia;
        Timestamp_t tempoMaximoManipulacao;
        double taxaOcupacaoMedia;
        int capacidadeMaximaUtilizada;
        
        EstatisticasSecao() : totalPacotesProcessados(0), tempoMedioPermanencia(0.0),
                             tempoMaximoManipulacao(0.0), taxaOcupacaoMedia(0.0),
                             capacidadeMaximaUtilizada(0) {}
    };
    
    class Secao {
    private:
        ID_t armazemDestino;
        Pilha<std::shared_ptr<Pacote>> pacotes;
        Distance_t tempoManipulacaoUnitario;
        Capacity_t ocupacaoMaxima;
        EstatisticasSecao estatisticas;
        
    public:
        Secao(ID_t destino, Capacity_t capacidade, Distance_t tempoManipulacao);
        
        // Operações de armazenamento
        bool armazenarPacote(std::shared_ptr<Pacote> pacote, Timestamp_t timestamp);
        std::shared_ptr<Pacote> recuperarPacote(size_t posicao, Timestamp_t timestamp);
        
        // Seleção para transporte
        std::vector<std::shared_ptr<Pacote>> selecionarPacotesMaisAntigos(
            Capacity_t quantidade, Timestamp_t timestamp);
        
        // Consultas
        bool vazia() const { return pacotes.vazia(); }
        bool cheia() const { return pacotes.cheia(); }
        size_t obterOcupacao() const { return pacotes.tamanho(); }
        Capacity_t obterCapacidadeMaxima() const { return ocupacaoMaxima; }
        
        Distance_t calcularTempoManipulacao(size_t posicao) const;
        std::shared_ptr<Pacote> obterPacoteMaisAntigo() const;
        
        // Estatísticas
        const EstatisticasSecao& obterEstatisticas() const { return estatisticas; }
        void atualizarEstatisticas(Timestamp_t timestampAtual);
        
        ID_t obterArmazemDestino() const { return armazemDestino; }
    };
    
    class Armazem : public IObservavel<std::shared_ptr<Pacote>> {
    private:
        ID_t id;
        std::string nome;
        std::unordered_map<ID_t, std::unique_ptr<Secao>> secoes;
        Capacity_t capacidadeTotal;
        
        // Estatísticas gerais
        int pacotesAtivos;
        double taxaOcupacao;
        std::vector<std::pair<Timestamp_t, double>> historicoOcupacao;
        
    public:
        Armazem(ID_t identificador, const std::string& nomeArmazem, 
                Capacity_t capacidade);
        
        // Configuração
        void adicionarSecao(ID_t armazemDestino, Capacity_t capacidadeSecao, 
                           Distance_t tempoManipulacao);
        void removerSecao(ID_t armazemDestino);
        
        // Operações principais
        bool receberPacote(std::shared_ptr<Pacote> pacote, Timestamp_t timestamp);
        std::vector<std::shared_ptr<Pacote>> prepararTransporte(
            ID_t armazemDestino, Capacity_t capacidade, Timestamp_t timestamp);
        
        // Consultas
        ID_t obterIdArmazem() const { return id; }
        const std::string& obterNome() const { return nome; }
        Capacity_t obterCapacidadeTotal() const { return capacidadeTotal; }
        int obterPacotesAtivos() const;
        double obterTaxaOcupacao() const;
        
        bool temSecao(ID_t armazemDestino) const;
        const Secao* obterSecao(ID_t armazemDestino) const;
        std::vector<ID_t> obterDestinosDisponiveis() const;
        
        bool estaVazio() const;
        bool temPacotesParaTransporte(ID_t destino) const;
        
        // Estatísticas
        void atualizarEstatisticas(Timestamp_t timestampAtual);
        std::unordered_map<ID_t, EstatisticasSecao> obterEstatisticasSecoes() const;
        
        // Análise de gargalos
        std::vector<ID_t> identificarSecoesSObrecarregadas(double threshold) const;
        Distance_t calcularTempoMedioManipulacao() const;
    };
}

#endif
