#include "Simulation.h"
#include <stdexcept>
#include <cstdlib>
#include <cstdio>

Simulation::Simulation(const std::string& inputFileName) 
    : capacidadeTransporte(0), latenciaTransporte(0), intervaloTransportes(0),
      custoRemocao(0), numeroArmazens(0), matrizAdjacencia(nullptr),
      armazens(nullptr), escalonador(nullptr), totalPacotes(0), pacotesEntregues(0) {

    std::ifstream arquivoEntrada(inputFileName);
    if (!arquivoEntrada.is_open()) {
        throw std::runtime_error("Erro: Nao foi possivel abrir o arquivo '" + inputFileName + "'.");
    }

    lerEntradaEAgendarChegadas(arquivoEntrada);
    arquivoEntrada.close();

    if (totalPacotes > 0 && escalonador != nullptr && !escalonador->vazio()) {
        Evento* primeiroEvento = escalonador->proximo();
        long tempoPrimeiraChegada = primeiroEvento->tempo;
        escalonador->agendar(primeiroEvento); 
        
        agendarTransportesIniciais(tempoPrimeiraChegada);
    }
}

Simulation::~Simulation() {
    if (matrizAdjacencia != nullptr) {
        for (int i = 0; i < numeroArmazens; ++i) {
            if (matrizAdjacencia[i] != nullptr) delete[] matrizAdjacencia[i];
        }
        delete[] matrizAdjacencia;
    }
    if (armazens != nullptr) {
        for (int i = 0; i < numeroArmazens; ++i) {
            if (armazens[i] != nullptr) delete armazens[i];
        }
        delete[] armazens;
    }
    if (escalonador != nullptr) delete escalonador;
}

void Simulation::run() {
    // A condição de parada é tratada dentro do próprio loop pela função simulacaoDeveTerminar()
    while (!simulacaoDeveTerminar()) {
        Evento* eventoAtual = escalonador->proximo();
        if (eventoAtual == nullptr) continue; // Segurança extra

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
}

void Simulation::lerEntradaEAgendarChegadas(std::ifstream& arquivoEntrada) {
    arquivoEntrada >> capacidadeTransporte >> latenciaTransporte >> intervaloTransportes >> custoRemocao;
    arquivoEntrada >> numeroArmazens;

    if (numeroArmazens <= 0) return;

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
    
    arquivoEntrada >> totalPacotes;
    
    // CORREÇÃO: Aumenta a capacidade do escalonador para evitar falhas silenciosas.
    escalonador = new Escalonador(totalPacotes * 2 + (numeroArmazens * numeroArmazens * 2) + 100);

    for (int i = 0; i < totalPacotes; ++i) {
        long tempo;
        int id_original, org, dst;
        std::string p, o, d;
        arquivoEntrada >> tempo >> p >> id_original >> o >> org >> d >> dst;

        Pacote* novoPacote = new Pacote{i, org, dst, tempo, nullptr, 0, 1};
        calcularRota(novoPacote);

        Evento* eventoChegada = new Evento{tempo, PACOTE_CHEGA, novoPacote, org, -1};
        escalonador->agendar(eventoChegada);
    }
}

void Simulation::agendarTransportesIniciais(long tempoPrimeiraChegada) {
    long tempoPrimeiroTransporte = tempoPrimeiraChegada + intervaloTransportes;
    for (int i = 0; i < numeroArmazens; ++i) {
        for (int j = 0; j < numeroArmazens; ++j) {
            if (matrizAdjacencia[i][j] == 1) {
                Evento* eventoTransporte = new Evento{tempoPrimeiroTransporte, INICIA_TRANSPORTE, nullptr, i, j};
                escalonador->agendar(eventoTransporte);
            }
        }
    }
}

bool Simulation::simulacaoDeveTerminar() const {
    // CORREÇÃO FINAL: A simulação termina quando o número de pacotes entregues (ou descartados)
    // alcança o número total de pacotes que entraram no sistema.
    return pacotesEntregues >= totalPacotes;
}

void Simulation::processaChegada(Evento* evento) {
    Pacote* pacote = evento->pacote;
    int armazemAtualId = evento->armazemOrigem;
    pacote->tempoPostagem = evento->tempo;

    if (armazemAtualId == pacote->destino) {
        printf("%07ld pacote %03d entregue em %03d\n", evento->tempo, pacote->id, armazemAtualId);
        pacotesEntregues++;
        delete[] pacote->rota;
        delete pacote;
    } else {
        // CORREÇÃO FINAL: Verifica a validade da rota ANTES de tentar armazenar.
        if (pacote->posRota >= pacote->tamRota) {
            // Se a rota é inválida, o pacote é impossível.
            // Deletamos e ajustamos a meta da simulação para evitar loops infinitos.
            delete[] pacote->rota;
            delete pacote;
            totalPacotes--;
        } else {
            // Se a rota é válida, armazena normalmente.
            armazens[armazemAtualId]->armazena(pacote);
        }
    }
}

void Simulation::processaTransporte(Evento* evento) {
    int origemId = evento->armazemOrigem;
    int destinoId = evento->armazemDestino;
    long tempoAtual = evento->tempo;

    Pilha<Pacote*>& secao = armazens[origemId]->getSecao(destinoId);

    if (!secao.isEmpty()) {
        int numPacotesNaSecao = secao.getTamanho();
        Pacote** listaParaAnalise = new Pacote*[numPacotesNaSecao];
        Node<Pacote*>* noAtual = secao.getTopo();
        for (int i = 0; i < numPacotesNaSecao; ++i) {
            listaParaAnalise[i] = noAtual->data;
            noAtual = noAtual->next;
        }

        qsort(listaParaAnalise, numPacotesNaSecao, sizeof(Pacote*), comparaPacotes);

        bool* ehAlvo = new bool[totalPacotes]();
        int numAlvos = (capacidadeTransporte < numPacotesNaSecao) ? capacidadeTransporte : numPacotesNaSecao;
        for (int i = 0; i < numAlvos; ++i) {
            ehAlvo[listaParaAnalise[i]->id] = true;
        }

        Pilha<Pacote*> pacotesRemovidos;
        int alvosEncontrados = 0;
        while (alvosEncontrados < numAlvos && !secao.isEmpty()) {
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
            if (ehAlvo[p->id]) { paraTransportar.push(p); } else { paraRearmazenar.push(p); }
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

    // CORREÇÃO: A lógica de reagendamento voltou ao original para não causar terminação prematura.
    if (pacotesEntregues < totalPacotes) {
        Evento* proximoTransporte = new Evento{tempoAtual + intervaloTransportes, INICIA_TRANSPORTE, nullptr, origemId, destinoId};
        escalonador->agendar(proximoTransporte);
    }
}

void Simulation::calcularRota(Pacote* pacote) {
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

int Simulation::comparaPacotes(const void* a, const void* b) {
    Pacote* pa = *(Pacote**)a;
    Pacote* pb = *(Pacote**)b;
    if (pa->tempoPostagem != pb->tempoPostagem) {
        return (pa->tempoPostagem < pb->tempoPostagem) ? -1 : 1;
    }
    // CORREÇÃO: Lógica de comparação mais segura para evitar overflow de inteiros.
    if (pa->id < pb->id) return -1;
    if (pa->id > pb->id) return 1;
    return 0;
}