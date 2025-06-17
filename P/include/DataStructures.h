#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <iostream>

// --- Declaração Avançada ---
struct Evento;

// --- Node for Linked Lists ---
template<typename T>
struct Node {
    T data;
    Node* next;
    Node(T val) : data(val), next(nullptr) {}
};

// --- Stack Implementation (LIFO) ---
// Como é uma classe de template, a implementação deve ficar no header.
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
        if (isEmpty()) return T();
        Node<T>* temp = topo;
        T val = temp->data;
        topo = topo->next;
        delete temp;
        tamanho--;
        return val;
    }
    bool isEmpty() const { return topo == nullptr; }
    int getTamanho() const { return tamanho; }
    Node<T>* getTopo() const { return topo; }
};

// --- Queue Implementation (FIFO) ---
// Como é uma classe de template, a implementação deve ficar no header.
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
        if (isEmpty()) return T();
        Node<T>* temp = frente;
        T val = temp->data;
        frente = frente->next;
        if (frente == nullptr) tras = nullptr;
        delete temp;
        return val;
    }
    bool isEmpty() const { return frente == nullptr; }
};

// --- Priority Queue (Min-Heap) Implementation ---
// Esta é uma classe concreta, então separamos declaração de implementação.
class FilaDePrioridade {
private:
    Evento** heap;
    int capacidade;
    int tamanho;
    void heapifyCima(int index);
    void heapifyBaixo(int index);
    
    // +++ CORREÇÃO: Implementação das funções auxiliares movida para o header +++
    int parent(int i) { return (i - 1) / 2; }
    int left(int i) { return 2 * i + 1; }
    int right(int i) { return 2 * i + 2; }

    void swap(int i, int j);
    bool comparaEventos(Evento* a, Evento* b);
public:
    // Apenas declarações dos métodos
    FilaDePrioridade(int cap);
    ~FilaDePrioridade();
    void insere(Evento* evento);
    Evento* removeMin();
    bool isEmpty() const;
};

#endif // DATA_STRUCTURES_H
