#include <iostream>
#include <string>
#include <cstdlib>
#include <sstream>
#include <fstream> 

#include "Types.h"
#include "DataStructures.h"

// ... (variáveis globais e funções 'comparaPacotes' e 'calcularRota' não mudam) ...
int capacidadeTransporte;
int latenciaTransporte;
int intervaloTransportes;
int custoRemocao;
int numeroArmazens;
int** matrizAdjacencia = nullptr;
Armazem** armazens = nullptr;
Escalonador* escalonador = nullptr;
int totalPacotes;
int pacotesEntregues = 0;

int comparaPacotes(const void* a, const void* b) {
    Pacote* pa = *(Pacote**)a;
    Pacote* pb = *(Pacote**)b;
    if (pa->tempoPostagem != pb->tempoPostagem) {
        return (pa->tempoPostagem < pb->tempoPostagem) ? -1 : 1;
    }
    return pa->id - pb->id;
}

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


// +++ FUNÇÃO DE LEITURA MODIFICADA para receber um fluxo de arquivo +++
long lerEntradaEAgendarChegadas(std::ifstream& arquivoEntrada) {
    std::string linha;

    arquivoEntrada >> capacidadeTransporte >> latenciaTransporte >> intervaloTransportes >> custoRemocao;
    arquivoEntrada >> numeroArmazens;

    if (numeroArmazens <= 0) return -1;

    matrizAdjacencia = new int*[numeroArmazens];
    for (int i = 0; i < numeroArmazens; ++i) {
        matrizAdjacencia[i] = new int[numeroArmazens];
        for (int j = 0; j < numeroArmazens; ++j) {
            arquivoEntrada >> matrizAdjacencia[i][j];
        }
    }

    armazens = new Armazem*[numeroArmazens];
    for (int i = 0; i < numeroArmazens; ++i) {
        armazens[i] = new Armazem(i, numeroArmazens);
    }
    
    int numPacotes = 0;
    arquivoEntrada >> numPacotes;
    totalPacotes = numPacotes;
    
    escalonador = new Escalonador(numPacotes + (numeroArmazens * numeroArmazens * 2) + 1);
    long minTempoChegada = -1;

    for (int i = 0; i < numPacotes; ++i) {
        long tempo;
        int id_original, org, dst;
        std::string p, o, d;
        arquivoEntrada >> tempo >> p >> id_original >> o >> org >> d >> dst;

        if (minTempoChegada == -1 || tempo < minTempoChegada) {
            minTempoChegada = tempo;
        }

        Pacote* novoPacote = new Pacote{i, org, dst, tempo, nullptr, 0, 1};
        calcularRota(novoPacote);

        Evento* eventoChegada = new Evento{tempo, PACOTE_CHEGA, novoPacote, org, -1};
        escalonador->agendar(eventoChegada);
    }
    return minTempoChegada;
}

void processaTransporte(Evento* evento) {
    int origemId = evento->armazemOrigem;
    int destinoId = evento->armazemDestino;
    long tempoAtual = evento->tempo;

    Pilha<Pacote*>& secao = armazens[origemId]->getSecao(destinoId);

    if (!secao.isEmpty()) {
        int numPacotesNaSecao = secao.getTamanho();
        Pacote** listaParaAnalise = new Pacote*[numPacotesNaSecao];
        Node<Pacote*>* noAtual = secao.getTopo();
        for(int i = 0; i < numPacotesNaSecao; ++i) {
            listaParaAnalise[i] = noAtual->data;
            noAtual = noAtual->next;
        }
        
        qsort(listaParaAnalise, numPacotesNaSecao, sizeof(Pacote*), comparaPacotes);

        bool* ehAlvo = new bool[totalPacotes]();
        int numAlvos = (capacidadeTransporte < numPacotesNaSecao) ? capacidadeTransporte : numPacotesNaSecao;
        for(int i = 0; i < numAlvos; ++i) {
            ehAlvo[listaParaAnalise[i]->id] = true;
        }
        
        Pilha<Pacote*> pacotesRemovidos;
        int alvosEncontrados = 0;
        
        while(alvosEncontrados < numAlvos && !secao.isEmpty()) {
            Pacote* p = secao.pop();
            pacotesRemovidos.push(p);
            if (ehAlvo[p->id]) {
                alvosEncontrados++;
            }
        }

        long tempoLog = tempoAtual;
        Pilha<Pacote*> paraTransportar;
        Pilha<Pacote*> paraRearmazenar;

        Pilha<Pacote*> pilhaOrdemImpressao;
        while(!pacotesRemovidos.isEmpty()){
            pilhaOrdemImpressao.push(pacotesRemovidos.pop());
        }

        while(!pilhaOrdemImpressao.isEmpty()){
            Pacote* p = pilhaOrdemImpressao.pop();
            tempoLog += custoRemocao;
            printf("%07ld pacote %03d removido de %03d na secao %03d\n", tempoLog, p->id, origemId, destinoId);
            
            if (ehAlvo[p->id]) {
                paraTransportar.push(p);
            } else {
                paraRearmazenar.push(p);
            }
        }
        
        while(!paraTransportar.isEmpty()) {
            Pacote* p = paraTransportar.pop();
            printf("%07ld pacote %03d em transito de %03d para %03d\n", tempoLog, p->id, origemId, destinoId);
            Evento* eventoChegada = new Evento{tempoLog + latenciaTransporte, PACOTE_CHEGA, p, destinoId, -1};
            escalonador->agendar(eventoChegada);
        }

        while(!paraRearmazenar.isEmpty()) {
            Pacote* p = paraRearmazenar.pop();
            printf("%07ld pacote %03d rearmazenado em %03d na secao %03d\n", tempoLog, p->id, origemId, destinoId);
            secao.push(p); 
        }

        delete[] listaParaAnalise;
        delete[] ehAlvo;
    }

    if (pacotesEntregues < totalPacotes) {
        Evento* proximoTransporte = new Evento{tempoAtual + intervaloTransportes, INICIA_TRANSPORTE, nullptr, origemId, destinoId};
        escalonador->agendar(proximoTransporte);
    }
}

bool simulacaoDeveTerminar() {
    if (totalPacotes > 0 && pacotesEntregues >= totalPacotes) {
        return true;
    }
    if (totalPacotes == 0) {
        return escalonador->vazio();
    }
    if(escalonador->vazio()){
        for (int i = 0; i < numeroArmazens; i++) {
            if (armazens != nullptr && armazens[i] != nullptr && !armazens[i]->secoesVazias()) {
                return false;
            }
        }
    }
    return escalonador->vazio();
}

void cleanup() {
    if (matrizAdjacencia != nullptr) {
        for (int i = 0; i < numeroArmazens; ++i) {
            if(matrizAdjacencia[i] != nullptr) delete[] matrizAdjacencia[i];
        }
        delete[] matrizAdjacencia;
        matrizAdjacencia = nullptr;
    }
    if (armazens != nullptr) {
         for (int i = 0; i < numeroArmazens; ++i) {
            if(armazens[i] != nullptr) {
                for (int j = 0; j < numeroArmazens; ++j) {
                    Pilha<Pacote*>& secao = armazens[i]->getSecao(j);
                    while (!secao.isEmpty()) {
                        Pacote* p = secao.pop();
                        delete[] p->rota; // Deleta a rota do pacote
                        delete p;         // Deleta o objeto do pacote
                    }
                }
                delete armazens[i]; // Deleta o próprio armazém
            }
        }
        delete[] armazens;
        armazens = nullptr;
    }

    if(escalonador != nullptr) delete escalonador;
    escalonador = nullptr;
}

int main(int argc, char *argv[]) {
    // Verifica se o nome do arquivo foi passado como argumento
    if (argc < 2) {
        // fprintf(stderr, "Erro: Nenhum arquivo de entrada especificado.\nUso: ./bin/tp2.out <arquivo_de_entrada>\n");
        return 1; // Retorna erro
    }

    // Abre o arquivo especificado no argumento argv[1]
    std::ifstream arquivoEntrada(argv[1]);
    if (!arquivoEntrada.is_open()) {
        // fprintf(stderr, "Erro: Não foi possível abrir o arquivo '%s'.\n", argv[1]);
        return 1; // Retorna erro
    }


    long tempoPrimeiraChegada = lerEntradaEAgendarChegadas(arquivoEntrada);

    // Fecha o arquivo após a leitura
    arquivoEntrada.close();

    if (tempoPrimeiraChegada == -1) { 
        cleanup(); 
        return 0;  
    }

    if (totalPacotes == 0) {
        cleanup();
        return 0;
    }

    long tempoPrimeiroTransporte = tempoPrimeiraChegada + intervaloTransportes;
    for (int i = 0; i < numeroArmazens; ++i) {
        for (int j = 0; j < numeroArmazens; ++j) {
            if (matrizAdjacencia[i][j] == 1) {
                Evento* eventoTransporte = new Evento{tempoPrimeiroTransporte, INICIA_TRANSPORTE, nullptr, i, j};
                escalonador->agendar(eventoTransporte);
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