#ifndef TYPES_H
#define TYPES_H

#include <iostream>

template<typename T> class Pilha;
class FilaDePrioridade;

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

#endif // TYPES_H