#ifndef REDE_ARMAZENS_HPP
#define REDE_ARMAZENS_HPP

#include "estruturas/Grafo.hpp" //
#include "entidades/Armazem.hpp" //
#include <unordered_map>
#include <memory>
#include <vector>

namespace LogisticSystem {

    class RedeArmazens {
    private:
        std::shared_ptr<Grafo> grafoArmazens; //
        std::unordered_map<ID_t, std::shared_ptr<Armazem>> armazens; //

    public:
        RedeArmazens();
        ~RedeArmazens() = default;

        bool adicionarArmazem(ID_t id, const std::string& nome, Capacity_t capacidadeTotal);
        bool removerArmazem(ID_t id);
        std::shared_ptr<Armazem> obterArmazem(ID_t id) const;
        std::vector<std::shared_ptr<Armazem>> obterTodosArmazens() const;
        
        std::shared_ptr<Grafo> obterGrafo() const { return grafoArmazens; } //

        void limpar();
    };

} // namespace LogisticSystem

#endif