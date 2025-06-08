#ifndef LEITOR_ARQUIVOS_HPP
#define LEITOR_ARQUIVOS_HPP

#include "entidades/RedeArmazens.hpp"
#include "entidades/SistemaTransporte.hpp"
#include "entidades/Pacote.hpp"
#include "utils/Tipos.hpp"
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

namespace LogisticSystem {

    class LeitorArquivos {
    public:
        // Assume que a rede de armazéns e o sistema de transporte são passados para
        // serem preenchidos, ou a função os retorna.
        // Optamos por passar smart pointers para serem modificados.
        static bool lerTopologia(const std::string& arquivo,
                                 std::shared_ptr<RedeArmazens> redeArmazens,
                                 std::shared_ptr<SistemaTransporte> sistemaTransporte,
                                 const ConfiguracaoSistema& configSistema);

        static std::vector<std::shared_ptr<Pacote>> lerPacotes(const std::string& arquivo);

    private:
        // Métodos auxiliares para parsing de linhas
        static std::vector<std::string> splitString(const std::string& s, char delimiter);
    };

} // namespace LogisticSystem

#endif