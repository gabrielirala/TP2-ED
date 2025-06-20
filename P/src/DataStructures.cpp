#include "DataStructures.h"
#include "Types.h"
#include <cstdio>

FilaDePrioridade::FilaDePrioridade(int cap) : capacidade(cap), tamanho(0) {
    heap = new Evento*[capacidade];
}

FilaDePrioridade::~FilaDePrioridade() {
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
        // Alerta para o caso de a fila de prioridades encher
        fprintf(stderr, "ALERTA: Fila de prioridades cheia! Capacidade = %d. Evento descartado.\n", capacidade);
        return;
    }
    tamanho++;
    heap[tamanho - 1] = evento;
    heapifyCima(tamanho - 1);
}

Evento* FilaDePrioridade::removeMin() {
    if (tamanho == 0) return nullptr;
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

bool FilaDePrioridade::comparaEventos(Evento* a, Evento* b) {
    // CORREÇÃO FINAL: Esta é a lógica de desempate mais robusta e padrão.
    // Prioridade 1: Tempo (menor primeiro)
    if (a->tempo != b->tempo) {
        return a->tempo < b->tempo;
    }

    // Prioridade 2: Tipo de Evento (menor enum primeiro)
    // Garante que chegadas sejam processadas antes de transportes no mesmo instante.
    if (a->tipo != b->tipo) {
        return a->tipo < b->tipo;
    }

    // Prioridade 3: Dados específicos como desempate final
    if (a->tipo == PACOTE_CHEGA) {
        return a->pacote->id < b->pacote->id;
    } else { // INICIA_TRANSPORTE
        if (a->armazemOrigem != b->armazemOrigem) {
            return a->armazemOrigem < b->armazemOrigem;
        }
        return a->armazemDestino < b->armazemDestino;
    }
}