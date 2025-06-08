#include "estruturas/Pilha.hpp"
#include <algorithm> // Para std::reverse

namespace LogisticSystem {

    template<typename T>
    const T& Pilha<T>::obterElemento(size_t posicao) const {
        if (posicao >= elementos.obterTamanho()) {
            throw std::out_of_range("Posicao invalida na pilha.");
        }
        // Em uma lista ligada que adiciona no início, a posição 0 é o topo.
        // Precisamos iterar para encontrar o elemento.
        auto it = elementos.begin();
        for (size_t i = 0; i < posicao; ++i) {
            ++it;
        }
        return *it;
    }

    template<typename T>
    Distance_t Pilha<T>::calcularTempoAcesso(size_t posicao, Distance_t tempoUnitario) const {
        if (posicao >= elementos.obterTamanho()) {
            throw std::out_of_range("Posicao invalida na pilha.");
        }
        // O tempo de acesso é proporcional à profundidade (posicao + 1)
        return (posicao + 1) * tempoUnitario;
    }

    template<typename T>
    std::vector<T> Pilha<T>::removerAteElemento(size_t posicao) {
        if (posicao >= elementos.obterTamanho()) {
            throw std::out_of_range("Posicao invalida: nao ha elementos suficientes para remover ate esta posicao.");
        }

        std::vector<T> removidos;
        // Remove elementos do topo até (e incluindo) a posicao desejada
        for (size_t i = 0; i <= posicao; ++i) {
            removidos.push_back(pop());
        }
        return removidos;
    }

    template<typename T>
    void Pilha<T>::recolocarElementos(const std::vector<T>& elementosARecolocar) {
        // Recoloca os elementos na pilha na ordem inversa que foram removidos
        // para manter a ordem LIFO original.
        // Se a remoção foi do topo para a base (posicao 0 até N),
        // eles devem ser recolocados da base para o topo.
        for (auto it = elementosARecolocar.rbegin(); it != elementosARecolocar.rend(); ++it) {
            push(*it);
        }
    }

    // AVISO: É crucial explicitamente instanciar os templates usados
    // ou incluir o .tpp se as implementações ficarem em um arquivo separado.
    // Para simplificar, neste contexto, estamos assumindo que as instâncias
    // serão feitas onde a Pilha for usada.
    // template class Pilha<std::shared_ptr<Pacote>>;

} // namespace LogisticSystem