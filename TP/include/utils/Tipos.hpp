#ifndef TIPOS_HPP
#define TIPOS_HPP

#include <string>
#include <vector> // Necessário para Capacidade_t e outras definições

namespace LogisticSystem {

    // Definições de tipos comuns para clareza
    using ID_t = int;            // Tipo para identificadores de armazéns, pacotes, etc.
    using Timestamp_t = double;  // Tipo para timestamps (pode ser tempo em horas, minutos, etc.)
    using Distance_t = double;   // Tipo para distâncias ou tempos de transporte
    using Capacity_t = int;      // Tipo para capacidades (quantidade de pacotes)

    // Estrutura para armazenar os parâmetros globais lidos do arquivo
    struct ParametrosSimulacaoGlobal {
        Capacity_t capacidadeTransporteGlobal;
        Distance_t latenciaTransporteGlobal;
        Timestamp_t intervaloTransportesGlobal;
        Distance_t custoRemocaoGlobal;
        Timestamp_t tempoFinalSimulacao; 
        bool modoDebug; 
        std::string arquivoSaida; 
    };

    // Enum para os estados de um pacote, conforme o novo enunciado (TP2 v1.1)
    enum class EstadoPacote {
        NAO_POSTADO,                // 1. Não foi postado
        CHEGADA_ESCALONADA,         // 2. Chegada escalonada a um armazém (postagem ou transporte)
        ARMAZENADO,                 // 3. Armazenado na seção associada ao próximo destino de um armazém
        REMOVIDO_PARA_TRANSPORTE,   // 4. Removido da seção para transporte (equivalente a ALOCADO_TRANSPORTE)
        ENTREGUE                    // 5. Entregue
        // O estado "CHEGOU_NAO_ARMAZENADO" foi removido.
        // O evento "rearmazenado" é uma *observação* de uma transição para ARMAZENADO.
    };

    // Estrutura para o histórico de estado de um pacote
    struct HistoricoEstado {
        Timestamp_t timestamp;
        EstadoPacote estado;
        ID_t armazemId; // ID do armazém onde ocorreu o evento
        std::string observacoes; // Detalhes adicionais sobre o evento (ex: "Pacote armazenado", "Pacote rearmazenado", "Pacote removido da seção para transporte")
    };

    // Enum para os tipos de evento no escalonador (conforme Tabela 1 do enunciado)
    enum class TipoEvento {
        PACOTE = 1,     // Evento relacionado a um pacote (ex: chegada, postagem)
        TRANSPORTE = 2  // Evento relacionado ao transporte entre armazéns
    };

    // Estrutura para dados de configuração do sistema (pode ser usada internamente ou para LeitorArquivos)
    struct ConfiguracaoSistema {
        Timestamp_t intervaloTransporte;
        Distance_t tempoManipulacaoUnitario;
        Distance_t tempoTransportePadrao;
        Capacity_t capacidadeTransportePadrao;
        double thresholdGargalo; // Se ainda for relevante para estatísticas
    };

} // namespace LogisticSystem

#endif // TIPOS_HPP
