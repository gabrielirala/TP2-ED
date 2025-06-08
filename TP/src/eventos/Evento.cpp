#include "eventos/Evento.hpp"

namespace LogisticSystem {

    Evento::Evento(TipoEvento tipoEvento, Timestamp_t tempo, int prio, const std::string& desc)
        : tipo(tipoEvento), timestamp(tempo), prioridade(prio), descricao(desc) {}
    // Métodos virtuais puros (executar, clonar, obterDetalhes)
    // serão implementados nas classes derivadas.

    bool Evento::operator<(const Evento& other) const {
        if (timestamp != other.timestamp) {
            return timestamp < other.timestamp;
        }
        return prioridade < other.prioridade;
    }

    bool Evento::operator>(const Evento& other) const {
        if (timestamp != other.timestamp) {
            return timestamp > other.timestamp;
        }
        return prioridade > other.prioridade;
    }

    bool Evento::operator==(const Evento& other) const {
        return tipo == other.tipo && timestamp == other.timestamp &&
               prioridade == other.prioridade && descricao == other.descricao;
    }

} // namespace LogisticSystem