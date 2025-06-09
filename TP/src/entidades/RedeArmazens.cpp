#include "entidades/RedeArmazens.hpp" //
#include <iostream>

namespace LogisticSystem {

    RedeArmazens::RedeArmazens() { //
        grafoArmazens = std::make_shared<Grafo>(true); // Grafo direcionado para a rede de armazéns
    }

    bool RedeArmazens::adicionarArmazem(ID_t id, const std::string& nome, Capacity_t capacidadeTotal) { //
        if (armazens.count(id)) {
            std::cerr << "Armazem com ID " << id << " ja existe." << std::endl;
            return false;
        }
        auto novoArmazem = std::make_shared<Armazem>(id, nome, capacidadeTotal); //
        armazens[id] = novoArmazem;
        grafoArmazens->adicionarVertice(id, nome); //
        std::cout << "Armazem " << nome << " (ID: " << id << ") adicionado." << std::endl;
        return true;
    }

    bool RedeArmazens::removerArmazem(ID_t id) { //
        if (!armazens.count(id)) {
            std::cerr << "Armazem com ID " << id << " nao encontrado para remocao." << std::endl;
            return false;
        }
        armazens.erase(id);
        grafoArmazens->removerVertice(id); //
        std::cout << "Armazem com ID " << id << " removido." << std::endl;
        return true;
    }

    std::shared_ptr<Armazem> RedeArmazens::obterArmazem(ID_t id) const { //
        auto it = armazens.find(id);
        if (it != armazens.end()) {
            return it->second;
        }
        return nullptr;
    }

    std::vector<std::shared_ptr<Armazem>> RedeArmazens::obterTodosArmazens() const { //
        std::vector<std::shared_ptr<Armazem>> listaArmazens;
        for (const auto& pair : armazens) {
            listaArmazens.push_back(pair.second);
        }
        return listaArmazens;
    }

    void RedeArmazens::limpar() { //
        armazens.clear();
        grafoArmazens->limpar(); //
        std::cout << "Rede de Armazens limpa." << std::endl;
    }

} // namespace LogisticSystem