#ifndef REDE_ARMAZENS_HPP
#define REDE_ARMAZENS_HPP

#include <map>
#include <string>
#include "Armazem.hpp"
#include "Produto.hpp"

class RedeArmazens {
private:
    std::map<std::string, Armazem*> armazens;

public:
    RedeArmazens();
    ~RedeArmazens();

    void adicionarArmazem(const std::string& nome, int capacidade);
    void removerArmazem(const std::string& nome);
    void adicionarProduto(const std::string& nomeArmazem, Produto* produto);
    void removerProduto(const std::string& nomeArmazem, const std::string& nomeProduto);
    Armazem* buscarArmazem(const std::string& nome);
    void listarArmazens() const;
    void listarProdutosArmazem(const std::string& nomeArmazem) const;
    int getTotalArmazens() const;
};

#endif // REDE_ARMAZENS_HPP