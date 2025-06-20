#include "Types.h"
#include "DataStructures.h"
#include <cstdio>

// --- Implementação dos Métodos do Armazem ---

Armazem::Armazem(int _id, int _numArmazens) 
    : id(_id), numTotalArmazens(_numArmazens) {
    secoes = new Pilha<Pacote*>[numTotalArmazens];
}

Armazem::~Armazem() {
    // CORREÇÃO: Garante que todos os pacotes restantes sejam deletados.
    for (int i = 0; i < numTotalArmazens; ++i) {
        while (!secoes[i].isEmpty()) {
            Pacote* p = secoes[i].pop();
            delete[] p->rota;
            delete p;
        }
    }
    delete[] secoes;
}

void Armazem::armazena(Pacote* pacote) {
    int proximoDestino = pacote->rota[pacote->posRota];
    secoes[proximoDestino].push(pacote);
    pacote->posRota++;
    
    printf("%07ld pacote %03d armazenado em %03d na secao %03d\n",
           pacote->tempoPostagem, pacote->id, id, proximoDestino);
}

Pilha<Pacote*>& Armazem::getSecao(int destino) {
    return secoes[destino];
}

bool Armazem::secoesVazias() const {
    for (int i = 0; i < numTotalArmazens; ++i) {
        if (!secoes[i].isEmpty()) {
            return false;
        }
    }
    return true;
}

// --- Implementação dos Métodos do Escalonador ---

Escalonador::Escalonador(int maxEventos) {
    pq = new FilaDePrioridade(maxEventos);
}

Escalonador::~Escalonador() {
    // CORREÇÃO: Garante que todos os eventos e pacotes restantes na fila sejam deletados.
    while (!pq->isEmpty()) {
        Evento* ev = pq->removeMin();
        if (ev->pacote != nullptr) {
            delete[] ev->pacote->rota;
            delete ev->pacote;
        }
        delete ev;
    }
    delete pq;
}

void Escalonador::agendar(Evento* evento) {
    pq->insere(evento);
}

Evento* Escalonador::proximo() {
    return pq->removeMin();
}

bool Escalonador::vazio() const {
    return pq->isEmpty();
}