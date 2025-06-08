#include "estruturas/Grafo.hpp"
#include <limits> // Para std::numeric_limits
#include <queue>  // Para std::priority_queue e std::queue
#include <algorithm> // Para std::reverse

namespace LogisticSystem {

    bool Grafo::adicionarVertice(ID_t id, const std::string& nome) {
        if (existeVertice(id)) {
            return false; // Vértice com este ID já existe
        }
        vertices[id] = std::make_unique<Vertice>(id, nome);
        return true;
    }

    bool Grafo::adicionarAresta(ID_t origem, ID_t destino, Distance_t peso, Capacity_t capacidade) {
        if (!existeVertice(origem) || !existeVertice(destino)) {
            return false; // Um dos vértices não existe
        }

        // Verifica se a aresta já existe
        for (const auto& aresta : vertices[origem]->adjacencias) {
            if (aresta.destino == destino) {
                return false; // Aresta já existe
            }
        }

        vertices[origem]->adjacencias.emplace_back(destino, peso, capacidade);
        if (!direcionado) {
            // Para grafos não direcionados, adiciona a aresta reversa
            vertices[destino]->adjacencias.emplace_back(origem, peso, capacidade);
        }
        return true;
    }

    bool Grafo::removerVertice(ID_t id) {
        if (!existeVertice(id)) {
            return false;
        }

        // Remove o vértice do mapa
        vertices.erase(id);

        // Remove todas as arestas que apontam para este vértice
        for (auto& pair : vertices) {
            auto& adj = pair.second->adjacencias;
            adj.erase(std::remove_if(adj.begin(), adj.end(),
                                     [id](const Aresta& a) { return a.destino == id; }),
                      adj.end());
        }
        return true;
    }

    bool Grafo::removerAresta(ID_t origem, ID_t destino) {
        if (!existeVertice(origem) || !existeVertice(destino)) {
            return false;
        }

        auto& adj_origem = vertices[origem]->adjacencias;
        bool removido = false;
        adj_origem.erase(std::remove_if(adj_origem.begin(), adj_origem.end(),
                                        [&](const Aresta& a) {
                                            if (a.destino == destino) {
                                                removido = true;
                                                return true;
                                            }
                                            return false;
                                        }),
                         adj_origem.end());

        if (!direcionado && removido) {
            // Se não direcionado e a aresta foi removida, remove a reversa
            auto& adj_destino = vertices[destino]->adjacencias;
            adj_destino.erase(std::remove_if(adj_destino.begin(), adj_destino.end(),
                                             [&](const Aresta& a) { return a.destino == origem; }),
                              adj_destino.end());
        }
        return removido;
    }

    bool Grafo::existeVertice(ID_t id) const {
        return vertices.count(id) > 0;
    }

    bool Grafo::existeAresta(ID_t origem, ID_t destino) const {
        if (!existeVertice(origem)) {
            return false;
        }
        for (const auto& aresta : vertices.at(origem)->adjacencias) {
            if (aresta.destino == destino) {
                return true;
            }
        }
        return false;
    }

    const Vertice* Grafo::obterVertice(ID_t id) const {
        if (!existeVertice(id)) {
            return nullptr;
        }
        return vertices.at(id).get();
    }

    Vertice* Grafo::obterVertice(ID_t id) {
        if (!existeVertice(id)) {
            return nullptr;
        }
        return vertices.at(id).get();
    }

    std::vector<ID_t> Grafo::obterVizinhos(ID_t vertice) const {
        std::vector<ID_t> vizinhos;
        const auto* v = obterVertice(vertice);
        if (v) {
            for (const auto& aresta : v->adjacencias) {
                vizinhos.push_back(aresta.destino);
            }
        }
        return vizinhos;
    }

    std::vector<Aresta> Grafo::obterArestas(ID_t vertice) const {
        const auto* v = obterVertice(vertice);
        if (v) {
            return v->adjacencias;
        }
        return {};
    }

    size_t Grafo::numeroArestas() const {
        size_t count = 0;
        for (const auto& pair : vertices) {
            count += pair.second->adjacencias.size();
        }
        return direcionado ? count : count / 2;
    }

    std::vector<ID_t> Grafo::buscarMenorCaminho(ID_t origem, ID_t destino) const {
        if (!existeVertice(origem) || !existeVertice(destino)) {
            return {}; // Caminho inválido se origem ou destino não existem
        }

        std::unordered_map<ID_t, Distance_t> distancias;
        std::unordered_map<ID_t, ID_t> predecessores;
        // priority_queue armazena pares {distancia, vertice_id}
        // A ordem é invertida para min-heap: menor distancia tem maior prioridade
        std::priority_queue<std::pair<Distance_t, ID_t>,
                            std::vector<std::pair<Distance_t, ID_t>>,
                            std::greater<std::pair<Distance_t, ID_t>>> fila_prioridade;

        for (const auto& pair : vertices) {
            distancias[pair.first] = std::numeric_limits<Distance_t>::infinity();
        }
        distancias[origem] = 0;
        fila_prioridade.push({0, origem});

        while (!fila_prioridade.empty()) {
            Distance_t dist_atual = fila_prioridade.top().first;
            ID_t u = fila_prioridade.top().second;
            fila_prioridade.pop();

            if (dist_atual > distancias[u]) {
                continue;
            }

            if (u == destino) {
                break; // Chegou ao destino
            }

            const auto* vertice_u = obterVertice(u);
            if (!vertice_u) continue; // Should not happen if vertices are valid

            for (const auto& aresta : vertice_u->adjacencias) {
                ID_t v = aresta.destino;
                Distance_t peso_aresta = aresta.peso;

                if (distancias[u] + peso_aresta < distancias[v]) {
                    distancias[v] = distancias[u] + peso_aresta;
                    predecessores[v] = u;
                    fila_prioridade.push({distancias[v], v});
                }
            }
        }

        // Reconstruir o caminho
        std::vector<ID_t> caminho;
        if (distancias[destino] == std::numeric_limits<Distance_t>::infinity()) {
            return {}; // Não há caminho
        }

        ID_t atual = destino;
        while (atual != origem) {
            caminho.push_back(atual);
            if (predecessores.find(atual) == predecessores.end()) {
                // Caminho incompleto (erro ou destino inatingível da origem)
                return {};
            }
            atual = predecessores[atual];
        }
        caminho.push_back(origem);
        std::reverse(caminho.begin(), caminho.end());
        return caminho;
    }

    std::vector<ID_t> Grafo::buscarCaminhoAlternativo(ID_t origem, ID_t destino,
                                                      const std::unordered_set<ID_t>& verticesProibidos) const {
        if (!existeVertice(origem) || !existeVertice(destino)) {
            return {};
        }
        if (verticesProibidos.count(origem) || verticesProibidos.count(destino)) {
            return {}; // Origem ou destino proibidos
        }

        std::unordered_map<ID_t, Distance_t> distancias;
        std::unordered_map<ID_t, ID_t> predecessores;
        std::priority_queue<std::pair<Distance_t, ID_t>,
                            std::vector<std::pair<Distance_t, ID_t>>,
                            std::greater<std::pair<Distance_t, ID_t>>> fila_prioridade;

        for (const auto& pair : vertices) {
            distancias[pair.first] = std::numeric_limits<Distance_t>::infinity();
        }
        distancias[origem] = 0;
        fila_prioridade.push({0, origem});

        while (!fila_prioridade.empty()) {
            Distance_t dist_atual = fila_prioridade.top().first;
            ID_t u = fila_prioridade.top().second;
            fila_prioridade.pop();

            if (dist_atual > distancias[u]) {
                continue;
            }

            if (u == destino) {
                break;
            }

            const auto* vertice_u = obterVertice(u);
            if (!vertice_u) continue;

            for (const auto& aresta : vertice_u->adjacencias) {
                ID_t v = aresta.destino;
                if (verticesProibidos.count(v)) {
                    continue; // Pula vértices proibidos
                }
                Distance_t peso_aresta = aresta.peso;

                if (distancias[u] + peso_aresta < distancias[v]) {
                    distancias[v] = distancias[u] + peso_aresta;
                    predecessores[v] = u;
                    fila_prioridade.push({distancias[v], v});
                }
            }
        }

        // Reconstruir o caminho
        std::vector<ID_t> caminho;
        if (distancias[destino] == std::numeric_limits<Distance_t>::infinity()) {
            return {}; // Não há caminho alternativo
        }

        ID_t atual = destino;
        while (atual != origem) {
            caminho.push_back(atual);
            if (predecessores.find(atual) == predecessores.end()) {
                return {};
            }
            atual = predecessores[atual];
        }
        caminho.push_back(origem);
        std::reverse(caminho.begin(), caminho.end());
        return caminho;
    }

    Distance_t Grafo::calcularDistanciaTotal(const std::vector<ID_t>& caminho) const {
        if (caminho.size() < 2) {
            return 0.0;
        }

        Distance_t distancia_total = 0.0;
        for (size_t i = 0; i < caminho.size() - 1; ++i) {
            ID_t u = caminho[i];
            ID_t v = caminho[i+1];
            bool aresta_encontrada = false;

            if (!existeVertice(u)) return -1.0; // Vértice não existe

            for (const auto& aresta : vertices.at(u)->adjacencias) {
                if (aresta.destino == v) {
                    distancia_total += aresta.peso;
                    aresta_encontrada = true;
                    break;
                }
            }
            if (!aresta_encontrada) {
                return -1.0; // Caminho inválido (não há aresta entre u e v)
            }
        }
        return distancia_total;
    }

    bool Grafo::validarCaminho(const std::vector<ID_t>& caminho) const {
        if (caminho.size() < 2) {
            return true; // Um caminho de 0 ou 1 vértice é trivialmente válido
        }

        for (size_t i = 0; i < caminho.size() - 1; ++i) {
            ID_t u = caminho[i];
            ID_t v = caminho[i+1];
            if (!existeAresta(u, v)) {
                return false; // Aresta não existe
            }
        }
        return true;
    }

    bool Grafo::ehConexo() const {
        if (vertices.empty()) {
            return true; // Um grafo vazio é considerado conexo
        }

        std::unordered_set<ID_t> visitados;
        std::queue<ID_t> fila;

        // Começa a BFS a partir do primeiro vértice no mapa
        ID_t start_node = vertices.begin()->first;
        fila.push(start_node);
        visitados.insert(start_node);

        while (!fila.empty()) {
            ID_t u = fila.front();
            fila.pop();

            const auto* vertice_u = obterVertice(u);
            if (!vertice_u) continue;

            for (const auto& aresta : vertice_u->adjacencias) {
                ID_t v = aresta.destino;
                if (visitados.find(v) == visitados.end()) {
                    visitados.insert(v);
                    fila.push(v);
                }
            }
        }

        return visitados.size() == vertices.size();
    }

    std::vector<std::vector<ID_t>> Grafo::obterComponentesConexos() const {
        std::vector<std::vector<ID_t>> componentes;
        std::unordered_set<ID_t> visitados_globais;

        for (const auto& pair : vertices) {
            ID_t vertice_atual = pair.first;
            if (visitados_globais.find(vertice_atual) == visitados_globais.end()) {
                std::vector<ID_t> componente_atual;
                std::queue<ID_t> fila_bfs;
                std::unordered_set<ID_t> visitados_componente;

                fila_bfs.push(vertice_atual);
                visitados_componente.insert(vertice_atual);
                visitados_globais.insert(vertice_atual);

                while (!fila_bfs.empty()) {
                    ID_t u = fila_bfs.front();
                    fila_bfs.pop();
                    componente_atual.push_back(u);

                    const auto* vertice_u = obterVertice(u);
                    if (!vertice_u) continue;

                    for (const auto& aresta : vertice_u->adjacencias) {
                        ID_t v = aresta.destino;
                        if (visitados_componente.find(v) == visitados_componente.end()) {
                            visitados_componente.insert(v);
                            visitados_globais.insert(v);
                            fila_bfs.push(v);
                        }
                    }
                }
                componentes.push_back(componente_atual);
            }
        }
        return componentes;
    }

    std::unordered_map<ID_t, int> Grafo::calcularGraus() const {
        std::unordered_map<ID_t, int> graus;
        for (const auto& pair : vertices) {
            graus[pair.first] = 0;
        }

        for (const auto& pair : vertices) {
            // Grau de saída
            graus[pair.first] += pair.second->adjacencias.size();

            // Se for não direcionado, o grau de entrada é igual ao de saída
            // Se for direcionado, precisa contar as arestas que chegam
            if (direcionado) {
                for (const auto& inner_pair : vertices) {
                    for (const auto& aresta : inner_pair.second->adjacencias) {
                        if (aresta.destino == pair.first) {
                            graus[pair.first]++;
                        }
                    }
                }
            }
        }
        return graus;
    }

    std::vector<ID_t> Grafo::obterTodosVertices() const {
        std::vector<ID_t> todos_vertices;
        for (const auto& pair : vertices) {
            todos_vertices.push_back(pair.first);
        }
        return todos_vertices;
    }

    void Grafo::limpar() {
        vertices.clear();
    }

} // namespace LogisticSystem