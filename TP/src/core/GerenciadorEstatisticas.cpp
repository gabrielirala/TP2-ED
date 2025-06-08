#include "core/GerenciadorEstatisticas.hpp"
#include <iostream>
#include <iomanip> // Para std::fixed, std::setprecision

namespace LogisticSystem {

    GerenciadorEstatisticas::GerenciadorEstatisticas(std::shared_ptr<Escalonador> esc,
                                                     std::shared_ptr<RedeArmazens> rede,
                                                     std::shared_ptr<SistemaTransporte> sist,
                                                     const std::vector<std::shared_ptr<Pacote>>& pacotes)
        : escalonador(esc), redeArmazens(rede), sistemaTransporte(sist), todosPacotes(pacotes) {}

    void GerenciadorEstatisticas::coletarEstatisticas() {
        // Coletar estatísticas do escalonador
        if (escalonador) {
            relatorioFinal.estatisticasEscalonador = escalonador->obterEstatisticas();
            relatorioFinal.tempoSimulacaoFinal = escalonador->obterTempoAtual();
        }

        // Coletar estatísticas dos armazéns
        if (redeArmazens) {
            auto armazens = redeArmazens->obterTodosArmazens();
            for (const auto& armazem_pair : armazens) {
                relatorioFinal.estatisticasArmazens[armazem_pair.first] = armazem_pair.second->obterEstatisticasSecoes();
            }
        }

        // Coletar estatísticas do sistema de transporte
        if (sistemaTransporte) {
            relatorioFinal.estatisticasRotas = sistemaTransporte->obterEstatisticasTodasRotas();
            relatorioFinal.eficienciaGeralTransporte = sistemaTransporte->calcularEficienciaGeral();
        }

        // Coletar métricas de pacotes entregues
        relatorioFinal.metricasPacotesCompletos.clear();
        for (const auto& pacote : todosPacotes) {
            if (pacote->obterEstadoAtual() == EstadoPacote::ENTREGUE) {
                relatorioFinal.metricasPacotesCompletos.push_back(pacote->calcularMetricas());
            }
        }
    }

    void GerenciadorEstatisticas::gerarRelatorioTexto(const std::string& arquivoSaida) const {
        std::ofstream outfile(arquivoSaida);
        if (!outfile.is_open()) {
            std::cerr << "Erro: Nao foi possivel criar o arquivo de relatorio: " << arquivoSaida << std::endl;
            return;
        }

        outfile << "--- Relatorio da Simulacao Logistica ---\n";
        outfile << "Tempo de Simulacao Final: " << std::fixed << std::setprecision(2) << relatorioFinal.tempoSimulacaoFinal << " unidades de tempo\n\n";

        // Estatísticas do Escalonador
        outfile << "--- Estatisticas do Escalonador ---\n";
        outfile << "Eventos Processados: " << relatorioFinal.estatisticasEscalonador.eventosProcessados << "\n";
        outfile << "Eventos de Chegada: " << relatorioFinal.estatisticasEscalonador.eventosChegada << "\n";
        outfile << "Eventos de Transporte: " << relatorioFinal.estatisticasEscalonador.eventosTransporte << "\n";
        outfile << "Eventos Descartados: " << relatorioFinal.estatisticasEscalonador.eventosDescartados << "\n";
        outfile << "Tempo Medio de Processamento de Evento: " << std::fixed << std::setprecision(4) << relatorioFinal.estatisticasEscalonador.tempoMedioProcessamento << "s\n\n";

        // Estatísticas dos Armazéns
        outfile << "--- Estatisticas dos Armazens ---\n";
        for (const auto& armazem_entry : relatorioFinal.estatisticasArmazens) {
            ID_t armazem_id = armazem_entry.first;
            auto armazem_ptr = redeArmazens->obterArmazem(armazem_id);
            if (!armazem_ptr) continue;
            outfile << "Armazem ID: " << armazem_id << " (" << armazem_ptr->obterNome() << ")\n";
            for (const auto& secao_entry : armazem_entry.second) {
                ID_t destino_secao_id = secao_entry.first;
                const EstatisticasSecao& stats = secao_entry.second;
                outfile << "  Secao para Destino " << destino_secao_id << ":\n";
                outfile << "    Total Pacotes Processados: " << stats.totalPacotesProcessados << "\n";
                outfile << "    Tempo Medio de Permanencia (Estimado): " << std::fixed << std::setprecision(2) << stats.tempoMedioPermanencia << "\n";
                outfile << "    Taxa de Ocupacao Media: " << std::fixed << std::setprecision(2) << (stats.taxaOcupacaoMedia * 100.0) << "%\n";
                outfile << "    Capacidade Maxima Utilizada: " << stats.capacidadeMaximaUtilizada << "\n";
            }
            outfile << "\n";
        }

        // Estatísticas das Rotas de Transporte
        outfile << "--- Estatisticas do Sistema de Transporte ---\n";
        outfile << "Eficiencia Geral do Transporte: " << std::fixed << std::setprecision(2) << (relatorioFinal.eficienciaGeralTransporte * 100.0) << "%\n";
        for (const auto& rota_entry : relatorioFinal.estatisticasRotas) {
            const std::string& chave_rota = rota_entry.first;
            const EstatisticasRota& stats = rota_entry.second;
            outfile << "  Rota " << chave_rota << ":\n";
            outfile << "    Viagens Realizadas: " << stats.viagensRealizadas << "\n";
            outfile << "    Total Pacotes Transportados: " << stats.totalPacotesTransportados << "\n";
            outfile << "    Taxa de Utilizacao Media: " << std::fixed << std::setprecision(2) << (stats.taxaUtilizacaoMedia * 100.0) << "%\n";
            outfile << "    Capacidade Media Utilizada: " << stats.capacidadeMediaUtilizada << "\n";
            outfile << "    Tempo Medio de Viagem: " << std::fixed << std::setprecision(2) << stats.tempoMedioViagem << "\n";
        }
        outfile << "\n";

        // Métricas de Pacotes Entregues
        outfile << "--- Metricas dos Pacotes Entregues (" << relatorioFinal.metricasPacotesCompletos.size() << ")\n";
        for (const auto& metricas : relatorioFinal.metricasPacotesCompletos) {
            outfile << "  - Tempo Esperado: " << std::fixed << std::setprecision(2) << metricas.tempoEsperado
                    << ", Tempo Armazenado: " << metricas.tempoArmazenado
                    << ", Tempo Transito: " << metricas.tempoTransito
                    << ", Atraso Total: " << metricas.atrasoTotal
                    << ", Transferencias: " << metricas.numeroTransferencias
                    << ", Gargalo Detectado: " << (metricas.gargaloDetectado ? "Sim" : "Nao") << "\n";
        }
        outfile << "\n";

        outfile << "--- Fim do Relatorio ---\n";
        outfile.close();
        std::cout << "Relatorio gerado com sucesso em: " << arquivoSaida << std::endl;
    }

    void GerenciadorEstatisticas::salvarRelatorioBinario(const std::string& arquivo) const {
        // Implementação para salvar o relatório em formato binário (serialização)
        // Isso pode ser complexo e requer uma biblioteca de serialização (ex: Boost.Serialization)
        // ou implementação manual. Para este protótipo, será deixado vazio.
        std::cerr << "Funcionalidade de salvar relatorio binario nao implementada." << std::endl;
    }

} // namespace LogisticSystem