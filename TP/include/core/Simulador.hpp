#ifndef SIMULADOR_HPP
#define SIMULADOR_HPP

#include "Escalonador.hpp"
#include "GerenciadorEstatisticas.hpp"
#include "entidades/RedeArmazens.hpp"
#include "entidades/SistemaTransporte.hpp"
#include "utils/Tipos.hpp"
#include "interfaces/IProcessadorEvento.hpp"
#include <memory>
#include <string>
#include <vector>

namespace LogisticSystem {
    struct ParametrosSimulacao {
        Timestamp_t tempoFinal;
        Distance_t tempoTransportePadrao;
        Distance_t tempoManipulacaoPadrao;
        Capacity_t capacidadeTransportePadrao;
        Distance_t intervaloTransporte;
        double thresholdGargalo;
        bool modoDebug;
        std::string arquivoTopologia;
        std::string arquivoPacotes;
        std::string arquivoSaida;
        
        ParametrosSimulacao() : tempoFinal(0.0), tempoTransportePadrao(2.5),
                              tempoManipulacaoPadrao(0.1), capacidadeTransportePadrao(10),
                              intervaloTransporte(24.0), thresholdGargalo(1.5),
                              modoDebug(false) {}
    };
    
    class Simulador : public IProcessadorEvento {
    private:
        // Componentes principais
        std::unique_ptr<Escalonador> escalonador;
        std::unique_ptr<RedeArmazens> redeArmazens;
        std::unique_ptr<SistemaTransporte> sistemaTransporte;
        std::unique_ptr<GerenciadorEstatisticas> gerenciadorEstatisticas;
        
        // Configuração
        ParametrosSimulacao parametros;
        bool simulacaoInicializada;
        bool simulacaoFinalizada;
        
        // Dados de entrada
        std::vector<std::shared_ptr<Pacote>> pacotes;
        
    public:
        Simulador();
        ~Simulador();
        
        // Configuração da simulação
        void carregarParametros(const std::string& arquivoConfig);
        void carregarParametros(const ParametrosSimulacao& params);
        void definirArquivoTopologia(const std::string& arquivo);
        void definirArquivoPacotes(const std::string& arquivo);
        void definirArquivoSaida(const std::string& arquivo);
        
        // Inicialização
        bool inicializar();
        void reinicializar();
        
        // Execução
        bool executarSimulacao();
        void executarAteTimestamp(Timestamp_t limite);
        void executarProximoEvento();
        void pararSimulacao();
        
        // IProcessadorEvento
        void processarEvento(std::shared_ptr<Evento> evento) override;
        bool podeProcessar(TipoEvento tipo) const override;
        
        // Consultas de estado
        bool estaInicializada() const { return simulacaoInicializada; }
        bool estaFinalizada() const { return simulacaoFinalizada; }
        Timestamp_t obterTempoAtual() const;
        
        // Resultados
        void gerarRelatorios() const;
        void salvarEstatisticas() const;
        void salvarEstatisticas(const std::string& arquivo) const;
        
        // Componentes (acesso controlado)
        const Escalonador* obterEscalonador() const { return escalonador.get(); }
        const RedeArmazens* obterRedeArmazens() const { return redeArmazens.get(); }
        const SistemaTransporte* obterSistemaTransporte() const { return sistemaTransporte.get(); }
        const GerenciadorEstatisticas* obterGerenciadorEstatisticas() const { return gerenciadorEstatisticas.get(); }
        
    private:
        // Métodos de inicialização
        bool carregarTopologia();
        bool carregarPacotes();
        void configurarSistemas();
        void agendarEventosIniciais();
        
        // Métodos de execução
        bool verificarCondicaoFinalizacao();
        void finalizarSimulacao();
        void limparRecursos();
        
        // Validação
        bool validarParametros() const;
        bool validarArquivos() const;
        bool validarConsistencia() const;
    };
}

#endif
