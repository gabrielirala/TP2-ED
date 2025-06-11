#ifndef TYPES_H
#define TYPES_H

// --- Main Abstract Data Types (ADTs) --- 

// Represents the type of event in the simulation
enum TipoEvento {
    PACOTE_CHEGA,
    INICIA_TRANSPORTE
};

// TAD Pacote: Represents a package and its journey 
struct Pacote {
    int id;
    int origem;
    int destino;
    long tempoPostagem;
    
    // Route is a dynamically allocated array of warehouse IDs 
    int* rota;
    int tamRota;
    int posRota; // Current index in the 'rota' array
};

// TAD Evento: Represents an event to be processed by the scheduler 
struct Evento {
    long tempo;
    TipoEvento tipo;
    
    // Event-specific data
    Pacote* pacote;
    int armazemOrigem;
    int armazemDestino;
};


// Forward declaration of Pilha
template<typename T> class Pilha;

// TAD Armazem: Manages package storage in sections 
class Armazem {
private:
    int id;
    // Each section is a stack of packages, indexed by destination warehouse ID 
    Pilha<Pacote*>* secoes;
    int numTotalArmazens;

public:
    Armazem(int id, int numArmazens);
    ~Armazem();
    
    // Places a package in the correct section stack 
    void armazena(Pacote* pacote);
    
    // Retrieves the stack for a given destination
    Pilha<Pacote*>& getSecao(int destino);

    int getId() const;
    bool secoesVazias() const;
};


// Forward declaration of FilaDePrioridade
class FilaDePrioridade;

// TAD Escalonador: The central component for discrete event simulation 
class Escalonador {
private:
    // A priority queue to manage events chronologically 
    FilaDePrioridade* pq;

public:
    Escalonador(int maxEventos);
    ~Escalonador();
    
    // Schedules a new event 
    void agendar(Evento* evento);
    
    // Retrieves the next event to be executed 
    Evento* proximo();
    
    bool vazio() const;
};

#endif // TYPES_H