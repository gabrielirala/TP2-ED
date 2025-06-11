#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include "Types.h" // Forward declaration of Evento
#include <iostream>

// Forward declaration to resolve circular dependency for Evento comparison
struct Evento;

// --- Node for Linked Lists ---
template<typename T>
struct Node {
    T data;
    Node* next;

    Node(T val) : data(val), next(nullptr) {}
};

// --- Stack Implementation (LIFO) ---
// Used for warehouse sections 
template<typename T>
class Pilha {
private:
    Node<T>* topo;
    int tamanho;

public:
    Pilha() : topo(nullptr), tamanho(0) {}

    ~Pilha() {
        while (!isEmpty()) {
            pop();
        }
    }

    void push(T val) {
        Node<T>* novoNode = new Node<T>(val);
        novoNode->next = topo;
        topo = novoNode;
        tamanho++;
    }

    T pop() {
        if (isEmpty()) {
            // In a real scenario, throw an exception
            return T();
        }
        Node<T>* temp = topo;
        T val = temp->data;
        topo = topo->next;
        delete temp;
        tamanho--;
        return val;
    }

    T peek() {
        if (isEmpty()) {
            // In a real scenario, throw an exception
            return T();
        }
        return topo->data;
    }

    bool isEmpty() const {
        return topo == nullptr;
    }

    int getTamanho() const {
        return tamanho;
    }
};

// --- Queue Implementation (FIFO) ---
// Used for Breadth-First Search (BFS) to find package routes 
template<typename T>
class Fila {
private:
    Node<T>* frente;
    Node<T>* tras;

public:
    Fila() : frente(nullptr), tras(nullptr) {}

    ~Fila() {
        while (!isEmpty()) {
            dequeue();
        }
    }

    void enqueue(T val) {
        Node<T>* novoNode = new Node<T>(val);
        if (isEmpty()) {
            frente = tras = novoNode;
        } else {
            tras->next = novoNode;
            tras = novoNode;
        }
    }

    T dequeue() {
        if (isEmpty()) {
            // In a real scenario, throw an exception
            return T();
        }
        Node<T>* temp = frente;
        T val = temp->data;
        frente = frente->next;
        if (frente == nullptr) {
            tras = nullptr;
        }
        delete temp;
        return val;
    }

    bool isEmpty() const {
        return frente == nullptr;
    }
};


// --- Priority Queue (Min-Heap) Implementation ---
// The core of the discrete event scheduler 
class FilaDePrioridade {
private:
    Evento** heap;
    int capacidade;
    int tamanho;

    void heapifyCima(int index) {
        while (index > 0 && comparaEventos(heap[index], heap[parent(index)])) {
            swap(index, parent(index));
            index = parent(index);
        }
    }

    void heapifyBaixo(int index) {
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

    int parent(int i) { return (i - 1) / 2; }
    int left(int i) { return 2 * i + 1; }
    int right(int i) { return 2 * i + 2; }

    void swap(int i, int j) {
        Evento* temp = heap[i];
        heap[i] = heap[j];
        heap[j] = temp;
    }
    
    // Custom comparator to establish total ordering of events 
    bool comparaEventos(Evento* a, Evento* b);

public:
    FilaDePrioridade(int cap) : capacidade(cap), tamanho(0) {
        heap = new Evento*[capacidade];
    }

    ~FilaDePrioridade() {
        for(int i = 0; i < tamanho; ++i) {
            delete heap[i];
        }
        delete[] heap;
    }

    void insere(Evento* evento) {
        if (tamanho == capacidade) {
            // Resize logic would be needed for a fully dynamic heap
            return;
        }
        tamanho++;
        heap[tamanho - 1] = evento;
        heapifyCima(tamanho - 1);
    }

    Evento* removeMin() {
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

    bool isEmpty() const {
        return tamanho == 0;
    }
};

#endif // DATA_STRUCTURES_H