#ifndef FILA_PRIORIDADE_HPP
#define FILA_PRIORIDADE_HPP

#include <vector>
#include <stdexcept>

template <typename T>
class FilaPrioridade {
private:
    std::vector<T> heap;

    void siftUp(int index) {
        while (index > 0) {
            int parent = (index - 1) / 2;
            if (heap[parent] < heap[index]) {
                std::swap(heap[parent], heap[index]);
                index = parent;
            } else {
                break;
            }
        }
    }

    void siftDown(int index) {
        int size = heap.size();
        while (true) {
            int largest = index;
            int left = 2 * index + 1;
            int right = 2 * index + 2;

            if (left < size && heap[left] > heap[largest]) {
                largest = left;
            }
            if (right < size && heap[right] > heap[largest]) {
                largest = right;
            }

            if (largest != index) {
                std::swap(heap[index], heap[largest]);
                index = largest;
            } else {
                break;
            }
        }
    }

public:
    FilaPrioridade() = default;

    void inserir(const T& elemento) {
        heap.push_back(elemento);
        siftUp(heap.size() - 1);
    }

    T removerMaximo() {
        if (vazia()) {
            throw std::runtime_error("Fila de prioridade vazia");
        }

        T max = heap[0];
        heap[0] = heap.back();
        heap.pop_back();

        if (!vazia()) {
            siftDown(0);
        }

        return max;
    }

    const T& maximo() const {
        if (vazia()) {
            throw std::runtime_error("Fila de prioridade vazia");
        }
        return heap[0];
    }

    bool vazia() const {
        return heap.empty();
    }

    size_t tamanho() const {
        return heap.size();
    }

    void limpar() {
        heap.clear();
    }
};

#endif // FILA_PRIORIDADE_HPP