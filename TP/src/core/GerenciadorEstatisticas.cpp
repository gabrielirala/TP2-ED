#include "core/GerenciadorEstatisticas.hpp"
#include <iostream>
#include <iomanip>   // Para std::setw, std::setfill, std::fixed, std::setprecision
#include <algorithm> // Para std::sort
#include <sstream>   // Para std::ostringstream

namespace LogisticSystem {

    // Hash para std::pair<ID_t, ID_t> necessário para unordered_map de estatisticasRotas
    struct PairHash {
        template <class T1, class T2>
        std::size_t operator () (const std::pair<T1, T2> &p) const {
            auto h1 = std::hash<T1>{}(p.first);
            auto h2 = std::hash<T2>{}(p.second);
            // Combina os hashes (pode ser ajustado para melhor distribuição)
            return h1 ^ (h2 << 1);
        }
    };

    GerenciadorEstatisticas::GerenciadorEstatisticas(
        std::shared_ptr<Escalonador> escalonador,
        std::shared_ptr<RedeArmazens> redeArmazens,
        std::shared_ptr<SistemaTransporte> sistemaTransporte,
        const std::vector<std::shared_ptr<Pacote>>& pacotes)
        : escalonador(escalonador), redeArmazens(redeArmazens), sistemaTransporte(sistemaTransporte),
          todosPacotes(pacotes) { // Copia a lista de shared_ptr de pacotes
        // Os mapas de estatísticas são inicializados por padrão
    }

    void GerenciadorEstatisticas::inicializar(
        std::shared_ptr<Escalonador> escalonador_ptr,
        std::shared_ptr<RedeArmazens> redeArmazens_ptr,
        std::shared_ptr<SistemaTransporte> sistemaTransporte_ptr,
        const std::vector<std::shared_ptr<Pacote>>& pacotes_list) {
        
        escalonador = escalonador_ptr;
        redeArmazens = redeArmazens_ptr;
        sistemaTransporte = sistemaTransporte_ptr;
        todosPacotes = pacotes_list; // Garante que a lista de pacotes está atualizada
        
        // Limpar estatísticas anteriores se houver (para reinicializações)
        estatisticasEscalonador = EstatisticasEscalonador();
        estatisticasArmazensSecoes.clear();
        estatisticasRotas.clear();
    }

    void GerenciadorEstatisticas::coletarEstatisticasEscalonador() {
        estatisticasEscalonador.totalEventosProcessados = escalonador->obterTotalEventosProcessados();
        estatisticasEscalonador.tempoSimulacaoTotal = escalonador->obterTempoAtual();
        // ... outras estatísticas do escalonador
    }

    void GerenciadorEstatisticas::coletarEstatisticasArmazens() {
        // Itera sobre todos os armazéns na rede
        for (const auto& armazem_pair : redeArmazens->obterTodosArmazens()) {
            ID_t armazemId = armazem_pair.first;
            const auto& armazem = armazem_pair.second;

            // Coleta estatísticas de cada seção dentro do armazém
            std::unordered_map<ID_t, EstatisticasSecao> secoesStats = armazem->obterEstatisticasSecoes();
            for (const auto& secao_pair : secoesStats) {
                // Combina o ID do armazém e da seção para a chave do mapa de estatísticas
                // Se o ID da seção já é único globalmente, pode usar direto secao_pair.first
                // Se não, você pode usar uma chave composta: std::make_pair(armazemId, secao_pair.first)
                estatisticasArmazensSecoes[armazemId] = secao_pair.second; // Exemplo: armazenando por ID do armazem
            }
        }
    }

    void GerenciadorEstatisticas::coletarEstatisticasSistemaTransporte() {
        // Coleta estatísticas de cada rota de transporte
        // O SistemaTransporte deve ter um método para obter suas estatísticas de rota
        for (const auto& rota_entry : sistemaTransporte->obterEstatisticasTodasRotas()) {
            estatisticasRotas[rota_entry.first] = rota_entry.second;
        }
        // ... outras estatísticas gerais do sistema de transporte (eficiência, etc.)
    }

    void GerenciadorEstatisticas::coletarEstatisticasPacotes() {
        // As estatísticas internas dos pacotes (tempo armazenado, em trânsito)
        // são atualizadas diretamente pelos métodos do Pacote (`Pacote::atualizarEstado`)
        // ao longo da simulação.
        // Aqui, iteramos para garantir que todos os pacotes chamaram `calcularMetricas`
        // se essa função finaliza algum cálculo.
        for (const auto& pacote : todosPacotes) {
            pacote->calcularMetricas(); // Garante que as métricas finais estão atualizadas
        }
    }

    void GerenciadorEstatisticas::coletarEstatisticas() {
        coletarEstatisticasEscalonador();
        coletarEstatisticasArmazens();
        coletarEstatisticasSistemaTransporte();
        coletarEstatisticasPacotes(); // Garante que métricas do pacote estão finalizadas
    }

    // Função auxiliar para formatar IDs e tempos com zeros à esquerda
    std::string formatID(ID_t id, int width) {
        std::ostringstream oss;
        oss << std::setw(width) << std::setfill('0') << id;
        return oss.str();
    }

    // Gera o relatório de texto com a sequência de eventos de cada pacote
    void GerenciadorEstatisticas::gerarRelatorioTexto(const std::string& nomeArquivoSaida) const {
        std::ofstream arquivoSaida(nomeArquivoSaida);
        if (!arquivoSaida.is_open()) {
            std::cerr << "Erro: Nao foi possivel abrir o arquivo de saida: " << nomeArquivoSaida << std::endl;
            return;
        }

        // Estrutura temporária para coletar e ordenar todos os eventos de todos os pacotes
        struct EventoParaImpressao {
            Timestamp_t timestamp;
            ID_t pacoteId;
            EstadoPacote estado;
            ID_t armazemId; // Onde o evento ocorreu (origem para trânsito)
            std::string observacoes;
            ID_t armazemOrigemTrans; // Para eventos de transito
            ID_t armazemDestinoTrans; // Para eventos de transito
        };

        std::vector<EventoParaImpressao> eventosOrdenados;
        for (const auto& pacote_ptr : todosPacotes) {
            ID_t pacote_id = pacote_ptr->obterIdUnico();
            for (const auto& historico_entry : pacote_ptr->obterHistorico()) {
                eventosOrdenados.push_back({
                    historico_entry.timestamp,
                    pacote_id, // Adiciona o ID do pacote
                    historico_entry.estado,
                    historico_entry.armazemId,
                    historico_entry.observacoes,
                    historico_entry.armazemOrigemTransporte,
                    historico_entry.armazemDestinoTransporte
                });
            }
        }

        // Ordenar todos os eventos por timestamp.
        // Em caso de timestamps iguais, ordenar por ID do pacote para consistência,
        // e depois pelo valor do enum do estado.
        std::sort(eventosOrdenados.begin(), eventosOrdenados.end(), [](const EventoParaImpressao& a, const EventoParaImpressao& b) {
            if (a.timestamp != b.timestamp) {
                return a.timestamp < b.timestamp;
            }
            if (a.pacoteId != b.pacoteId) {
                return a.pacoteId < b.pacoteId;
            }
            // Última ordem: pelo valor do enum do estado
            return static_cast<int>(a.estado) < static_cast<int>(b.estado);
        });

        // Iterar sobre os eventos ordenados e gerar a saída formatada
        for (const auto& evento_imp : eventosOrdenados) {
            // Formatar timestamp e IDs com zeros à esquerda
            // O tempo pode ser float/double, então converter para long long para preenchimento de zeros
            std::string tempoStr = formatID(static_cast<ID_t>(std::round(evento_imp.timestamp)), 7); // Arredonda para inteiro para formatação de tempo
            std::string pacoteIdStr = formatID(evento_imp.pacoteId, 3);
            
            arquivoSaida << tempoStr << " pacote " << pacoteIdStr;

            // Gerar a descrição do evento com base no estado e observações
            switch (evento_imp.estado) {
                case EstadoPacote::NAO_POSTADO:
                    // Geralmente não aparece na saída final como no exemplo, mas útil para debug.
                    // Se for para seguir estritamente o exemplo, este caso pode ser omitido.
                    arquivoSaida << " " << evento_imp.observacoes; // Ex: "Pacote criado, aguardando postagem."
                    break;
                case EstadoPacote::CHEGADA_ESCALONADA:
                    // Corresponde a um pacote chegando via postagem ou transporte em um armazém.
                    // No exemplo de saída, esses eventos são geralmente seguidos por "armazenado".
                    // Se a observação já contém a info completa, use-a. Senão, construa.
                    arquivoSaida << " chegou escalonada em " << formatID(evento_imp.armazemId, 3);
                    break;
                case EstadoPacote::ARMAZENADO:
                    // Pode ser "armazenado" ou "rearmazenado"
                    if (evento_imp.observacoes.find("Rearmazenado") != std::string::npos) {
                        arquivoSaida << " rearmazenado em " << formatID(evento_imp.armazemId, 3) << " na secao " << formatID(evento_imp.armazemId, 3);
                    } else {
                        arquivoSaida << " armazenado em " << formatID(evento_imp.armazemId, 3) << " na secao " << formatID(evento_imp.armazemId, 3);
                    }
                    break;
                case EstadoPacote::REMOVIDO_PARA_TRANSPORTE:
                    // Este estado corresponde ao evento "em transito" na saída.
                    // Assume que armazemOrigemTrans e armazemDestinoTrans foram preenchidos corretamente.
                    if (evento_imp.armazemOrigemTrans != -1 && evento_imp.armazemDestinoTrans != -1) {
                         arquivoSaida << " em transito de " << formatID(evento_imp.armazemOrigemTrans, 3)
                                      << " para " << formatID(evento_imp.armazemDestinoTrans, 3);
                    } else {
                         // Fallback se os IDs de transporte não foram preenchidos (deveriam ser)
                         arquivoSaida << " removido de " << formatID(evento_imp.armazemId, 3) << " na secao " << formatID(evento_imp.armazemId, 3);
                    }
                    break;
                case EstadoPacote::ENTREGUE:
                    arquivoSaida << " entregue em " << formatID(evento_imp.armazemId, 3);
                    break;
                default:
                    arquivoSaida << " estado desconhecido (" << static_cast<int>(evento_imp.estado) << ") em " << formatID(evento_imp.armazemId, 3);
                    break;
            }
            arquivoSaida << std::endl;
        }

        arquivoSaida.close();
        std::cout << "Relatorio de eventos gerado em: " << nomeArquivoSaida << std::endl;
    }

    void GerenciadorEstatisticas::salvarRelatorioBinario(const std::string& nomeArquivoSaida) const {
        std::cerr << "Funcionalidade de salvar relatorio binario nao implementada." << std::endl;
    }

} // namespace LogisticSystem
