#ifndef GERENCIADOR_ESTATISTICAS_HPP
#define GERENCIADOR_ESTATISTICAS_HPP

#include "utils/Tipos.hpp"
#include "entidades/Pacote.hpp"
#include "core/Escalonador.hpp"
#include "entidades/RedeArmazens.hpp"
#include "entidades/SistemaTransporte.hpp"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <fstream>

namespace LogisticSystem {

    struct RelatorioSimulacao {
        EstatisticasEscalonador estatisticasEscalonador;
        std::unordered_map<ID_t, std::unordered_map<ID_t, EstatisticasSecao>> estatisticasArmazens;
        std::unordered_map<std::string, EstatisticasRota> estatisticasRotas;
        std::vector<MetricasPacote> metricasPacotesCompletos;
        double eficienciaGeralTransporte;
        Timestamp_t tempoSimulacaoFinal;
        // Outras métricas globais podem ser adicionadas
    };

    class GerenciadorEstatisticas {
    private:
        std::shared_ptr<Escalonador> escalonador;
        std::shared_ptr<RedeArmazens> redeArmazens;
        std::shared_ptr<SistemaTransporte> sistemaTransporte;
        std::vector<std::shared_ptr<Pacote>> todosPacotes; // Referência a todos os pacotes

        RelatorioSimulacao relatorioFinal;

    public:
        GerenciadorEstatisticas(std::shared_ptr<Escalonador> esc,
                                std::shared_ptr<RedeArmazens> rede,
                                std::shared_ptr<SistemaTransporte> sist,
                                const std::vector<std::shared_ptr<Pacote>>& pacotes);

        void coletarEstatisticas();
        void gerarRelatorioTexto(const std::string& arquivoSaida) const;
        void salvarRelatorioBinario(const std::string& arquivo) const; // Opcional, para persistência
        const RelatorioSimulacao& obterRelatorioFinal() const { return relatorioFinal; }
    };

} // namespace LogisticSystem

#endif