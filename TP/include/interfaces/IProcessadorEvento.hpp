#ifndef IPROCESSADOR_EVENTO_HPP
#define IPROCESSADOR_EVENTO_HPP

#include "utils/Tipos.hpp"

namespace LogisticSystem {
    class Evento;
    
    class IProcessadorEvento {
    public:
        virtual ~IProcessadorEvento() = default;
        virtual void processarEvento(std::shared_ptr<Evento> evento) = 0;
        virtual bool podeProcessar(TipoEvento tipo) const = 0;
    };
}

#endif
