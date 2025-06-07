#ifndef GRAFO_HPP
#define GRAFO_HPP

#include "utils/Tipos.hpp"
#include "ListaLigada.hpp"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

namespace LogisticSystem {
    struct Aresta {
        ID_t destino;
        Distance_t peso;
        Capacity_t capacidade;
        
        Aresta(ID_t dest, Distance_t p, Capacity_t cap) 
            : destino(dest), peso(p), capacidade(cap) {}
    };
    
    struct Vertice {
        ID_t id;
        std::string nome;
        std::vector<Aresta> adjacencias;
        
        Vertice(ID_t identificador, const std::string& nomeVertice)
            : id(identificador), nome(nomeVertice) {}
    };
    
    class Grafo {
    private:
        std::unordered_map<ID_t, std::unique_ptr<Vertice>> vertices;
        bool direcionado;
        
    public:
        explicit Grafo(bool eh_direcionado = false) : direcionado(eh_direcionado) {}
        
        // Construção do grafo
        bool adicionarVertice(ID_t id, const std::string& nome);
        bool adicionarAresta(ID_t origem, ID_t destino, Distance_t peso, Capacity_t capacidade);
        bool removerVertice(ID_t id);
        bool removerAresta(ID_t origem, ID_t destino);
        
        // Consultas básicas
        bool existeVertice(ID_t id) const;
        bool existeAresta(ID_t origem, ID_t destino) const;
        const Vertice* obterVertice(ID_t id) const;
        Vertice* obterVertice(ID_t id);
        
        std::vector<ID_t> obterVizinhos(ID_t vertice) const;
        std::vector<Aresta> obterArestas(ID_t vertice) const;
        
        size_t numeroVertices() const { return vertices.size(); }
        size_t numeroArestas() const;
        
        // Algoritmos de busca
        std::vector<ID_t> buscarMenorCaminho(ID_t origem, ID_t destino) const;
        std::vector<ID_t> buscarCaminhoAlternativo(ID_t origem, ID_t destino, 
                                                  const std::unordered_set<ID_t>& verticesProibidos) const;
        
        Distance_t calcularDistanciaTotal(const std::vector<ID_t>& caminho) const;
        bool validarCaminho(const std::vector<ID_t>& caminho) const;
        
        // Análise do grafo
        bool ehConexo() const;
        std::vector<std::vector<ID_t>> obterComponentesConexos() const;
        std::unordered_map<ID_t, int> calcularGraus() const;
        
        // Operações de conjunto
        std::vector<ID_t> obterTodosVertices() const;
        
        void limpar();
    };
}

#endif
