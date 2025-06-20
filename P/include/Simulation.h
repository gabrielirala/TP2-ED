#ifndef SIMULATION_H
#define SIMULATION_H

#include <string>
#include <fstream>
#include "Types.h"
#include "DataStructures.h"

class Simulation {
public:
    Simulation(const std::string& inputFileName);
    ~Simulation();
    void run();

private:
    int capacidadeTransporte;
    int latenciaTransporte;
    int intervaloTransportes;
    int custoRemocao;
    int numeroArmazens;
    int** matrizAdjacencia;
    Armazem** armazens;
    Escalonador* escalonador;
    int totalPacotes;
    int pacotesEntregues;

    void lerEntradaEAgendarChegadas(std::ifstream& arquivoEntrada);
    void agendarTransportesIniciais(long tempoPrimeiraChegada);
    bool simulacaoDeveTerminar() const;

    void processaChegada(Evento* evento);
    void processaTransporte(Evento* evento);

    void calcularRota(Pacote* pacote);
    static int comparaPacotes(const void* a, const void* b);
};

#endif // SIMULATION_H