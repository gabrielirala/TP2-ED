#ifndef TYPES_H
#define TYPES_H

// --- Declarações Avançadas ---
// Informa ao compilador que estes tipos existem, sem precisar incluir DataStructures.h
template<typename T> class Pilha;
class FilaDePrioridade;

// --- Tipos Principais (ADTs) ---

enum TipoEvento {
    PACOTE_CHEGA,
    INICIA_TRANSPORTE
};

struct Pacote {
    int id;
    int origem;
    int destino;
    long tempoPostagem;
    int* rota;
    int tamRota;
    int posRota;
};

struct Evento {
    long tempo;
    TipoEvento tipo;
    Pacote* pacote;
    int armazemOrigem;
    int armazemDestino;
};

class Armazem {
private:
    int id;
    Pilha<Pacote*>* secoes;
    int numTotalArmazens;
public:
    Armazem(int id, int numArmazens);
    ~Armazem();
    void armazena(Pacote* pacote);
    Pilha<Pacote*>& getSecao(int destino);
    int getId() const;
    bool secoesVazias() const;
};

class Escalonador {
private:
    FilaDePrioridade* pq;
public:
    Escalonador(int maxEventos);
    ~Escalonador();
    void agendar(Evento* evento);
    Evento* proximo();
    bool vazio() const;
};

// --- Declaração de Funções Globais ---
void processaChegada(Evento* evento);

#endif // TYPES_H