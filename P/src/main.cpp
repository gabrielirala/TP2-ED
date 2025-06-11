#include <iostream>
#include <string>

#include "Types.h"
#include "DataStructures.h"

// Simulation Parameters 
int capacidadeTransporte;
int latenciaTransporte;
int intervaloTransportes;
int custoRemocao;
int numeroArmazens;
int** matrizAdjacencia;
Armazem** armazens;
Escalonador* escalonador;

int totalPacotes;
int pacotesEntregues = 0;

// --- Helper Functions ---

// Calculates the shortest path using Breadth-First Search (BFS) 
void calcularRota(Pacote* pacote) {
    int origem = pacote->origem;
    int destino = pacote->destino;

    Fila<int> fila;
    fila.enqueue(origem);

    int* antecessor = new int[numeroArmazens];
    bool* visitado = new bool[numeroArmazens];
    for (int i = 0; i < numeroArmazens; ++i) {
        antecessor[i] = -1;
        visitado[i] = false;
    }
    visitado[origem] = true;

    while (!fila.isEmpty()) {
        int u = fila.dequeue();
        if (u == destino) break;

        for (int v = 0; v < numeroArmazens; ++v) {
            if (matrizAdjacencia[u][v] == 1 && !visitado[v]) {
                visitado[v] = true;
                antecessor[v] = u;
                fila.enqueue(v);
            }
        }
    }

    // Reconstruct path
    Pilha<int> caminho;
    int atual = destino;
    while (atual != -1) {
        caminho.push(atual);
        atual = antecessor[atual];
    }

    pacote->tamRota = caminho.getTamanho();
    pacote->rota = new int[pacote->tamRota];
    for (int i = 0; i < pacote->tamRota; ++i) {
        pacote->rota[i] = caminho.pop();
    }

    delete[] antecessor;
    delete[] visitado;
}

void lerEntrada() {
    std::cin >> capacidadeTransporte >> latenciaTransporte >> intervaloTransportes >> custoRemocao;
    std::cin >> numeroArmazens;

    matrizAdjacencia = new int*[numeroArmazens];
    for (int i = 0; i < numeroArmazens; ++i) {
        matrizAdjacencia[i] = new int[numeroArmazens];
        for (int j = 0; j < numeroArmazens; ++j) {
            std::cin >> matrizAdjacencia[i][j];
        }
    }

    armazens = new Armazem*[numeroArmazens];
    for (int i = 0; i < numeroArmazens; ++i) {
        armazens[i] = new Armazem(i, numeroArmazens);
    }
    
    int numPacotes;
    std::cin >> numPacotes;
    totalPacotes = numPacotes;
    
    // Maximum possible events: arrivals + initial transports
    escalonador = new Escalonador(numPacotes + numeroArmazens * numeroArmazens);

    for (int i = 0; i < numPacotes; ++i) {
        long tempo;
        int id, org, dst;
        std::string p, o, d;
        std::cin >> tempo >> p >> id >> o >> org >> d >> dst;

        Pacote* novoPacote = new Pacote{id, org, dst, tempo, nullptr, 0, 0};
        calcularRota(novoPacote);

        Evento* eventoChegada = new Evento{tempo, PACOTE_CHEGA, novoPacote, org, -1};
        escalonador->agendar(eventoChegada);
    }
}

// --- Event Processing Functions ---

void processaChegada(Evento* evento) {
    Pacote* pacote = evento->pacote;
    int armazemAtualId = pacote->rota[pacote->posRota];
    pacote->tempoPostagem = evento->tempo;

    if (armazemAtualId == pacote->destino) {
        // Package has reached its final destination 
        printf("%07ld pacote %03d entregue em %03d\n",
               evento->tempo, pacote->id, armazemAtualId);
        pacotesEntregues++;
        delete[] pacote->rota;
        delete pacote;
    } else {
        // Package arrived at an intermediate warehouse, store it 
        armazens[armazemAtualId]->armazena(pacote);
    }
}

// This function implements the complex transport logic 
// It must "dig" for the oldest packages in a LIFO stack.
void processaTransporte(Evento* evento) {
    int origemId = evento->armazemOrigem;
    int destinoId = evento->armazemDestino;
    long tempoAtual = evento->tempo;

    Pilha<Pacote*>& secao = armazens[origemId]->getSecao(destinoId);

    if (!secao.isEmpty()) {
        // --- Logic to "dig" for the oldest packages ---
        
        // 1. Temporarily move all packages to find their posting order
        Pilha<Pacote*> tempPilha;
        while (!secao.isEmpty()) {
            tempPilha.push(secao.pop());
        }

        // 2. Identify packages to transport (oldest ones)
        Fila<Pacote*> pacotesParaTransportar;
        int count = 0;
        while (!tempPilha.isEmpty() && count < capacidadeTransporte) {
            pacotesParaTransportar.enqueue(tempPilha.pop());
            count++;
        }
        
        // Packages left in tempPilha must be re-stored
        Pilha<Pacote*> pacotesParaRearmazenar;
        while(!tempPilha.isEmpty()){
            pacotesParaRearmazenar.push(tempPilha.pop());
        }

        // 3. Process removals, transports, and re-storages
        long tempoLog = tempoAtual;

        // Simulate removal of packages that were on top (re-stored ones)
        while(!pacotesParaRearmazenar.isEmpty()){
            Pacote* p = pacotesParaRearmazenar.pop();
            tempoLog += custoRemocao;
            printf("%07ld pacote %03d removido de %03d na secao %03d\n",
                   tempoLog, p->id, origemId, destinoId);
        }

        // Simulate removal of packages that will be transported
        Fila<Pacote*> transportadosFila;
         while(!pacotesParaTransportar.isEmpty()){
            Pacote* p = pacotesParaTransportar.dequeue();
            tempoLog += custoRemocao;
            printf("%07ld pacote %03d removido de %03d na secao %03d\n",
                   tempoLog, p->id, origemId, destinoId);
            transportadosFila.enqueue(p);
        }

        // 4. Log transport and re-storage events, schedule future arrivals
        // All these events happen logically at the final calculated time `tempoLog`
        
        // Re-store packages that were not transported
        while(!pacotesParaRearmazenar.isEmpty()) {
            Pacote* p = pacotesParaRearmazenar.pop();
            secao.push(p);
             printf("%07ld pacote %03d rearmazenado em %03d na secao %03d\n",
                   tempoLog, p->id, origemId, destinoId);
        }

        // Schedule arrivals for transported packages
        while(!transportadosFila.isEmpty()) {
            Pacote* p = transportadosFila.dequeue();
            printf("%07ld pacote %03d em transito de %03d para %03d\n",
                   tempoLog, p->id, origemId, destinoId);
            
            Evento* eventoChegada = new Evento{tempoLog + latenciaTransporte, PACOTE_CHEGA, p, destinoId, -1};
            escalonador->agendar(eventoChegada);
        }
    }

    // Schedule the next transport event for this route 
    //Evento* proximoTransporte = new Evento{tempoAtual + intervaloTransportes, INICIA_TRANSPORTE, nullptr, origemId, destinoId};
    //escalonador->agendar(proximoTransporte);

    if (pacotesEntregues < totalPacotes) {
        Evento* proximoTransporte = new Evento{tempoAtual + intervaloTransportes, INICIA_TRANSPORTE, nullptr, origemId, destinoId};
        escalonador->agendar(proximoTransporte);
    }
}

// Checks if the simulation should end 
bool simulacaoDeveTerminar() {
    if (!escalonador->vazio()) {
        return false;
    }
    for (int i = 0; i < numeroArmazens; i++) {
        if (!armazens[i]->secoesVazias()) {
            return false;
        }
    }
    return true;
}

void cleanup() {
    for (int i = 0; i < numeroArmazens; ++i) {
        delete[] matrizAdjacencia[i];
        delete armazens[i];
    }
    delete[] matrizAdjacencia;
    delete[] armazens;
    delete escalonador;
}

// --- Main Simulation ---
int main() {
    // 1. Initialization Phase 
    lerEntrada();

    // Schedule initial transport events for all connections
    for (int i = 0; i < numeroArmazens; ++i) {
        for (int j = 0; j < numeroArmazens; ++j) {
            if (matrizAdjacencia[i][j] == 1) {
                Evento* eventoTransporte = new Evento{intervaloTransportes, INICIA_TRANSPORTE, nullptr, i, j};
                escalonador->agendar(eventoTransporte);
            }
        }
    }

    // 2. Simulation Loop 
    while (!simulacaoDeveTerminar()) {
        Evento* eventoAtual = escalonador->proximo();
        if (eventoAtual == nullptr) continue; // Should not happen if simulacaoDeveTerminar is correct

        switch (eventoAtual->tipo) {
            case PACOTE_CHEGA:
                processaChegada(eventoAtual);
                break;
            case INICIA_TRANSPORTE:
                processaTransporte(eventoAtual);
                break;
        }
        delete eventoAtual; // Clean up the processed event
    }

    // 3. Cleanup
    cleanup();

    return 0;
}