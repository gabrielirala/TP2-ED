#ifndef SIMULADOR_HPP
#define SIMULADOR_HPP

#include <memory>
#include <string>
#include <vector>
#include <iostream> // Para debug

#include "core/Escalonador.hpp"
#include "entidades/RedeArmazens.hpp"
#include "entidades/SistemaTransporte.hpp"
#include "core/GerenciadorEstatisticas.hpp"
#include "entidades/Pacote.hpp"
#include "interfaces/IProcessadorEvento.hpp"
#include "interfaces/IObservador.hpp"
#include "utils/Tipos.hpp" // Para ParametrosSimulacaoGlobal

namespace LogisticSystem {

    // A classe Simulador orquestra a simulação de eventos discretos.
    // Ela age como um processador de eventos e um observador para gerenciar pacotes.
    class Simulador : public IProcessadorEvento,
                      public IObservador<std::shared_ptr<Pacote>>,
                      public std::enable_shared_from_this<Simulador> { // Habilita shared_from_this para obter shared_ptr de 'this'
    private:
        std::unique_ptr<Escalonador> escalonador;
        std::unique_ptr<RedeArmazens> redeArmazens;
        std::unique_ptr<SistemaTransporte> sistemaTransporte;
        std::unique_ptr<GerenciadorEstatisticas> gerenciadorEstatisticas;
        std::vector<std::shared_ptr<Pacote>> pacotes; // Lista de todos os pacotes na simulação

        ParametrosSimulacaoGlobal parametrosSimulacao; // NOVOS parâmetros globais

        bool simulacaoInicializada;
        bool simulacaoFinalizada;

        // Métodos internos de inicialização e validação
        bool validarParametros() const;
        bool validarArquivos() const;
        bool validarConsistencia() const; // Verifica a consistência da rede e pacotes

        // Funções para carregar e configurar os componentes da simulação
        void configurarSistemas();
        void agendarEventosIniciais(); // Agenda eventos de chegada de pacotes e transporte

        bool verificarCondicaoFinalizacao(); // Verifica se a simulação deve terminar
        void finalizarSimulacao(); // Realiza ações de finalização

        void limparRecursos(); // Libera recursos alocados

    public:
        Simulador();
        ~Simulador();

        // Inicializa a simulação lendo de um único arquivo de entrada
        bool inicializar(const std::string& arquivoEntrada);
        void reinicializar(); // Reinicializa o simulador para uma nova execução

        // Métodos de controle da simulação
        bool executarSimulacao();
        void executarAteTimestamp(Timestamp_t limite);
        void executarProximoEvento();
        void pararSimulacao();

        // Implementação da interface IProcessadorEvento
        void processarEvento(std::shared_ptr<Evento> evento) override;
        bool podeProcessar(TipoEvento tipo) const override;

        // Implementação da interface IObservador (para Pacote)
        // Chamado quando um pacote tem seu estado atualizado
        void atualizar(std::shared_ptr<Pacote> pacote) override {
            // O Simulador pode reagir a atualizações de estado do pacote,
            // por exemplo, para fins de depuração ou para gerenciar estatísticas.
            if (parametrosSimulacao.modoDebug) {
                std::cout << "[OBSERVER] Pacote " << pacote->obterIdUnico()
                          << " atualizado para estado " << static_cast<int>(pacote->obterEstadoAtual())
                          << " no tempo " << pacote->obterTempoUltimaAtualizacao()
                          << ". " << pacote->obterObservacoesUltimaAtualizacao() << std::endl;
            }
        }

        // Métodos de acesso
        Timestamp_t obterTempoAtual() const;

        // Métodos para geração de relatórios
        void gerarRelatorios() const;
        void salvarEstatisticas() const;
        void salvarEstatisticas(const std::string& arquivo) const;
    };

} // namespace LogisticSystem

#endif // SIMULADOR_HPP
