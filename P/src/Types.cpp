#include "Types.h"
#include "DataStructures.h"

// --- Evento Comparison ---
// Implements the total ordering logic for the priority queue 
// Order: Time > Event Type > Other details for tie-breaking
bool FilaDePrioridade::comparaEventos(Evento* a, Evento* b) {
    if (a->tempo < b->tempo) return true;
    if (a->tempo > b->tempo) return false;

    // Tie-breaking: CHEGADA events have priority over TRANSPORTE at the same time
    if (a->tipo < b->tipo) return true;
    if (a->tipo > b->tipo) return false;

    // Further tie-breaking if necessary
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

// --- Armazem Implementation ---
Armazem::Armazem(int _id, int _numArmazens) 
    : id(_id), numTotalArmazens(_numArmazens) {
    secoes = new Pilha<Pacote*>[numTotalArmazens];
}

Armazem::~Armazem() {
    delete[] secoes;
}

void Armazem::armazena(Pacote* pacote) {
    // Determine the next warehouse in the package's route
    int proximoDestino = pacote->rota[pacote->posRota + 1];
    secoes[proximoDestino].push(pacote);
    pacote->posRota++;
    
    // Print storage event message 
    printf("%07ld pacote %03d armazenado em %03d na secao %03d\n",
           pacote->tempoPostagem, pacote->id, id, proximoDestino);
}

Pilha<Pacote*>& Armazem::getSecao(int destino) {
    return secoes[destino];
}

int Armazem::getId() const {
    return id;
}

bool Armazem::secoesVazias() const {
    for (int i = 0; i < numTotalArmazens; ++i) {
        if (!secoes[i].isEmpty()) {
            return false;
        }
    }
    return true;
}


// --- Escalonador Implementation ---
Escalonador::Escalonador(int maxEventos) {
    pq = new FilaDePrioridade(maxEventos);
}

Escalonador::~Escalonador() {
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