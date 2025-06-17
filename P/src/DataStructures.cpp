#include "DataStructures.h"
#include "Types.h" // Necessário para a definição completa de Evento

// --- Implementação dos Métodos da FilaDePrioridade ---

FilaDePrioridade::FilaDePrioridade(int cap) : capacidade(cap), tamanho(0) {
    heap = new Evento*[capacidade];
}

FilaDePrioridade::~FilaDePrioridade() {
    // A limpeza dos objetos Evento* é feita no loop principal do main.
    // Este destrutor apenas libera o array do heap.
    delete[] heap;
}

void FilaDePrioridade::swap(int i, int j) {
    Evento* temp = heap[i];
    heap[i] = heap[j];
    heap[j] = temp;
}

void FilaDePrioridade::heapifyCima(int index) {
    while (index > 0 && comparaEventos(heap[index], heap[parent(index)])) {
        swap(index, parent(index));
        index = parent(index);
    }
}

void FilaDePrioridade::heapifyBaixo(int index) {
    int minIndex = index;
    int esq = left(index);
    int dir = right(index);

    if (esq < tamanho && comparaEventos(heap[esq], heap[minIndex])) {
        minIndex = esq;
    }
    if (dir < tamanho && comparaEventos(heap[dir], heap[minIndex])) {
        minIndex = dir;
    }
    if (index != minIndex) {
        swap(index, minIndex);
        heapifyBaixo(minIndex);
    }
}

void FilaDePrioridade::insere(Evento* evento) {
    if (tamanho == capacidade) {
        // Para este projeto, a capacidade inicial é considerada suficiente.
        return;
    }
    tamanho++;
    heap[tamanho - 1] = evento;
    heapifyCima(tamanho - 1);
}

Evento* FilaDePrioridade::removeMin() {
    if (tamanho == 0) {
        return nullptr;
    }
    if (tamanho == 1) {
        tamanho--;
        return heap[0];
    }

    Evento* root = heap[0];
    heap[0] = heap[tamanho - 1];
    tamanho--;
    heapifyBaixo(0);
    return root;
}

bool FilaDePrioridade::isEmpty() const {
    return tamanho == 0;
}

// Implementação da função de comparação de eventos
bool FilaDePrioridade::comparaEventos(Evento* a, Evento* b) {
    if (a->tempo < b->tempo) return true;
    if (a->tempo > b->tempo) return false;

    if (a->tipo < b->tipo) return true;
    if (a->tipo > b->tipo) return false;

    if (a->tipo == PACOTE_CHEGA) {
        return a->pacote->id < b->pacote->id;
    }
    if (a->tipo == INICIA_TRANSPORTE) {
        if (a->armazemOrigem < b->armazemOrigem) return true;
        if (a->armazemOrigem > b->armazemOrigem) return false;
        return a->armazemDestino < b->armazemDestino;
    }
    
    return false;
}
