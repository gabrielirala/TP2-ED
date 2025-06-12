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

// +++ CORREÇÃO: Variáveis para controle de término e tempo inicial +++
int totalPacotes;
int pacotesEntregues = 0;

// --- Helper Functions ---

// Calcula a rota usando BFS (sem alterações)
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

// +++ CORREÇÃO: A função agora retorna o tempo da primeira chegada +++
long lerEntradaEAgendarChegadas() {
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
    totalPacotes = numPacotes; // Armazena o total
    
    escalonador = new Escalonador(numPacotes + (numeroArmazens * numeroArmazens * 2));

    long minTempoChegada = -1; // Para encontrar o tempo do primeiro pacote

    for (int i = 0; i < numPacotes; ++i) {
        long tempo;
        int id_original, org, dst; // O ID original será descartado
        std::string p, o, d;
        std::cin >> tempo >> p >> id_original >> o >> org >> d >> dst;

        // +++ CORREÇÃO: Encontra o menor tempo de chegada +++
        if (minTempoChegada == -1 || tempo < minTempoChegada) {
            minTempoChegada = tempo;
        }

        // +++ CORREÇÃO: Usa o índice 'i' como o novo ID do pacote +++
        Pacote* novoPacote = new Pacote{i, org, dst, tempo, nullptr, 0, 0};
        calcularRota(novoPacote);

        Evento* eventoChegada = new Evento{tempo, PACOTE_CHEGA, novoPacote, org, -1};
        escalonador->agendar(eventoChegada);
    }
    return minTempoChegada;
}

// --- Event Processing Functions ---

void processaChegada(Evento* evento) {
    Pacote* pacote = evento->pacote;
    int armazemAtualId = pacote->rota[pacote->posRota];
    pacote->tempoPostagem = evento->tempo;

    if (armazemAtualId == pacote->destino) {
        printf("%07ld pacote %03d entregue em %03d\n",
               evento->tempo, pacote->id, armazemAtualId);
        pacotesEntregues++; // Incrementa contador
        delete[] pacote->rota;
        delete pacote;
    } else {
        armazens[armazemAtualId]->armazena(pacote);
    }
}

void processaTransporte(Evento* evento) {
    int origemId = evento->armazemOrigem;
    int destinoId = evento->armazemDestino;
    long tempoAtual = evento->tempo;

    Pilha<Pacote*>& secao = armazens[origemId]->getSecao(destinoId);

    if (!secao.isEmpty()) {
        Pilha<Pacote*> tempPilha;
        while (!secao.isEmpty()) {
            tempPilha.push(secao.pop());
        }

        long tempoLog = tempoAtual;
        
        while (!tempPilha.isEmpty()) {
            Pacote* p = tempPilha.pop();
            tempoLog += custoRemocao;
            
            printf("%07ld pacote %03d removido de %03d na secao %03d\n",
                   tempoLog, p->id, origemId, destinoId);
            printf("%07ld pacote %03d em transito de %03d para %03d\n",
                   tempoLog, p->id, origemId, destinoId);
            
            Evento* eventoChegada = new Evento{tempoLog + latenciaTransporte, PACOTE_CHEGA, p, destinoId, -1};
            escalonador->agendar(eventoChegada);

            if (tempPilha.getTamanho() >= capacidadeTransporte) {
                 secao.push(p);
            }
        }
    }

    if (pacotesEntregues < totalPacotes) {
        Evento* proximoTransporte = new Evento{tempoAtual + intervaloTransportes, INICIA_TRANSPORTE, nullptr, origemId, destinoId};
        escalonador->agendar(proximoTransporte);
    }
}

// ... (simulacaoDeveTerminar e cleanup sem alterações) ...
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
    long tempoPrimeiraChegada = lerEntradaEAgendarChegadas();

    if (tempoPrimeiraChegada != -1) {
        for (int i = 0; i < numeroArmazens; ++i) {
            for (int j = 0; j < numeroArmazens; ++j) {
                if (matrizAdjacencia[i][j] == 1) {
                    // +++ CORREÇÃO: Usa o tempo da primeira chegada para agendar o transporte inicial +++
                    long tempoPrimeiroTransporte = tempoPrimeiraChegada + intervaloTransportes;
                    Evento* eventoTransporte = new Evento{tempoPrimeiroTransporte, INICIA_TRANSPORTE, nullptr, i, j};
                    escalonador->agendar(eventoTransporte);
                }
            }
        }
    }

    while (!simulacaoDeveTerminar()) {
        Evento* eventoAtual = escalonador->proximo();
        if (eventoAtual == nullptr) continue; 

        switch (eventoAtual->tipo) {
            case PACOTE_CHEGA:
                processaChegada(eventoAtual);
                break;
            case INICIA_TRANSPORTE:
                processaTransporte(eventoAtual);
                break;
        }
        delete eventoAtual; 
    }

    cleanup();
    return 0;
}