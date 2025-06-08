#include "estruturas/ListaLigada.hpp"
#include <iostream> // Para debugging, pode ser removido depois

namespace LogisticSystem {

    template<typename T>
    void ListaLigada<T>::inserirInicio(const T& valor) {
        auto novoNo = std::make_unique<No>(valor);
        if (vazia()) {
            primeiro = std::move(novoNo);
            ultimo = primeiro.get();
        } else {
            novoNo->proximo = std::move(primeiro);
            primeiro = std::move(novoNo);
        }
        tamanho++;
    }

    template<typename T>
    void ListaLigada<T>::inserirInicio(T&& valor) {
        auto novoNo = std::make_unique<No>(std::move(valor));
        if (vazia()) {
            primeiro = std::move(novoNo);
            ultimo = primeiro.get();
        } else {
            novoNo->proximo = std::move(primeiro);
            primeiro = std::move(novoNo);
        }
        tamanho++;
    }

    template<typename T>
    void ListaLigada<T>::inserirFim(const T& valor) {
        auto novoNo = std::make_unique<No>(valor);
        if (vazia()) {
            primeiro = std::move(novoNo);
            ultimo = primeiro.get();
        } else {
            ultimo->proximo = std::move(novoNo);
            ultimo = ultimo->proximo.get();
        }
        tamanho++;
    }

    template<typename T>
    void ListaLigada<T>::inserirFim(T&& valor) {
        auto novoNo = std::make_unique<No>(std::move(valor));
        if (vazia()) {
            primeiro = std::move(novoNo);
            ultimo = primeiro.get();
        } else {
            ultimo->proximo = std::move(novoNo);
            ultimo = ultimo->proximo.get();
        }
        tamanho++;
    }

    template<typename T>
    T ListaLigada<T>::removerInicio() {
        if (vazia()) {
            throw std::underflow_error("Lista vazia: nao foi possivel remover do inicio.");
        }
        T dados = std::move(primeiro->dados);
        primeiro = std::move(primeiro->proximo);
        if (!primeiro) { // Se a lista ficou vazia
            ultimo = nullptr;
        }
        tamanho--;
        return dados;
    }

    template<typename T>
    T ListaLigada<T>::removerFim() {
        if (vazia()) {
            throw std::underflow_error("Lista vazia: nao foi possivel remover do fim.");
        }
        if (tamanho == 1) {
            return removerInicio();
        }

        No* atual = primeiro.get();
        while (atual->proximo.get() != ultimo) {
            atual = atual->proximo.get();
        }
        T dados = std::move(ultimo->dados);
        ultimo = atual;
        ultimo->proximo.reset();
        tamanho--;
        return dados;
    }

    template<typename T>
    bool ListaLigada<T>::remover(const T& valor) {
        if (vazia()) {
            return false;
        }

        if (primeiro->dados == valor) {
            removerInicio();
            return true;
        }

        No* atual = primeiro.get();
        while (atual->proximo && !(atual->proximo->dados == valor)) {
            atual = atual->proximo.get();
        }

        if (atual->proximo) {
            if (atual->proximo.get() == ultimo) { // Se o nó a ser removido é o último
                ultimo = atual;
            }
            atual->proximo = std::move(atual->proximo->proximo);
            tamanho--;
            return true;
        }
        return false;
    }

    template<typename T>
    bool ListaLigada<T>::remover(std::function<bool(const T&)> predicado) {
        if (vazia()) {
            return false;
        }

        if (predicado(primeiro->dados)) {
            removerInicio();
            return true;
        }

        No* atual = primeiro.get();
        while (atual->proximo && !predicado(atual->proximo->dados)) {
            atual = atual->proximo.get();
        }

        if (atual->proximo) {
            if (atual->proximo.get() == ultimo) { // Se o nó a ser removido é o último
                ultimo = atual;
            }
            atual->proximo = std::move(atual->proximo->proximo);
            tamanho--;
            return true;
        }
        return false;
    }

    template<typename T>
    const T& ListaLigada<T>::obterPrimeiro() const {
        if (vazia()) {
            throw std::underflow_error("Lista vazia.");
        }
        return primeiro->dados;
    }

    template<typename T>
    const T& ListaLigada<T>::obterUltimo() const {
        if (vazia()) {
            throw std::underflow_error("Lista vazia.");
        }
        return ultimo->dados;
    }

    template<typename T>
    T& ListaLigada<T>::obterPrimeiro() {
        if (vazia()) {
            throw std::underflow_error("Lista vazia.");
        }
        return primeiro->dados;
    }

    template<typename T>
    T& ListaLigada<T>::obterUltimo() {
        if (vazia()) {
            throw std::underflow_error("Lista vazia.");
        }
        return ultimo->dados;
    }

    template<typename T>
    void ListaLigada<T>::paraCada(std::function<void(T&)> funcao) {
        No* atual = primeiro.get();
        while (atual) {
            funcao(atual->dados);
            atual = atual->proximo.get();
        }
    }

    template<typename T>
    void ListaLigada<T>::paraCada(std::function<void(const T&)> funcao) const {
        const No* atual = primeiro.get();
        while (atual) {
            funcao(atual->dados);
            atual = atual->proximo.get();
        }
    }

    template<typename T>
    template<typename Predicate>
    ListaLigada<T> ListaLigada<T>::filtrar(Predicate pred) const {
        ListaLigada<T> resultado;
        const No* atual = primeiro.get();
        while (atual) {
            if (pred(atual->dados)) {
                resultado.inserirFim(atual->dados);
            }
            atual = atual->proximo.get();
        }
        return resultado;
    }

    template<typename T>
    template<typename Func>
    auto ListaLigada<T>::mapear(Func func) -> ListaLigada<decltype(func(std::declval<T>()))> {
        ListaLigada<decltype(func(std::declval<T>()))> resultado;
        No* atual = primeiro.get();
        while (atual) {
            resultado.inserirFim(func(atual->dados));
            atual = atual->proximo.get();
        }
        return resultado;
    }

    template<typename T>
    void ListaLigada<T>::limpar() {
        primeiro.reset();
        ultimo = nullptr;
        tamanho = 0;
    }

    // AVISO: É crucial explicitamente instanciar os templates usados
    // ou incluir o .tpp se as implementações ficarem em um arquivo separado.
    // Para simplificar, neste contexto, estamos assumindo que as instâncias
    // serão feitas onde a ListaLigada for usada.
    // Ou pode-se adicionar instâncias explícitas para tipos conhecidos:
    // template class ListaLigada<std::shared_ptr<Pacote>>;
    // template class ListaLigada<int>;

} // namespace LogisticSystem