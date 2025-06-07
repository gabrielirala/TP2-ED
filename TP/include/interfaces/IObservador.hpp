#ifndef IOBSERVADOR_HPP
#define IOBSERVADOR_HPP

#include "utils/Tipos.hpp"

namespace LogisticSystem {
    template<typename T>
    class IObservador {
    public:
        virtual ~IObservador() = default;
        virtual void notificar(const T& evento) = 0;
    };
    
    template<typename T>
    class IObservavel {
    private:
        std::vector<std::weak_ptr<IObservador<T>>> observadores;
        
    public:
        virtual ~IObservavel() = default;
        
        void adicionarObservador(std::shared_ptr<IObservador<T>> observador) {
            observadores.push_back(observador);
        }
        
        void removerObservador(std::shared_ptr<IObservador<T>> observador) {
            observadores.erase(
                std::remove_if(observadores.begin(), observadores.end(),
                    [observador](const std::weak_ptr<IObservador<T>>& weak_obs) {
                        return weak_obs.lock() == observador;
                    }), 
                observadores.end());
        }
        
    protected:
        void notificarObservadores(const T& evento) {
            for (auto it = observadores.begin(); it != observadores.end();) {
                if (auto obs = it->lock()) {
                    obs->notificar(evento);
                    ++it;
                } else {
                    it = observadores.erase(it);
                }
            }
        }
    };
}

#endif
