#ifndef TIPOS_HPP
#define TIPOS_HPP

#include <string>
#include <vector>
#include <memory>
#include <chrono>

namespace LogisticSystem {
    using ID_t = int;
    using Timestamp_t = double;
    using Capacity_t = int;
    using Distance_t = double;
    
    enum class EstadoPacote {
        NAO_POSTADO = 1,
        CHEGADA_ESCALONADA = 2,
        CHEGOU_NAO_ARMAZENADO = 3,
        ARMAZENADO = 4,
        ALOCADO_TRANSPORTE = 5,
        ENTREGUE = 6
    };
    
    enum class TipoEvento {
        CHEGADA_PACOTE = 1,
        TRANSPORTE = 2,
        MANUTENCAO = 3
    };
    
    struct ConfiguracaoSistema {
        Distance_t tempoTransportePadrao;
        Distance_t tempoManipulacaoUnitario;
        Capacity_t capacidadeTransportePadrao;
        Distance_t intervaloTransporte;
        double thresholdGargalo;
    };
    
    struct MetricasPacote {
        Timestamp_t tempoEsperado;
        Timestamp_t tempoArmazenado;
        Timestamp_t tempoTransito;
        Timestamp_t atrasoTotal;
        int numeroTransferencias;
        bool gargaloDetectado;
    };
}

#endif
