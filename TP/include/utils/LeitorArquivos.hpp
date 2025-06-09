#ifndef LEITOR_ARQUIVOS_HPP
#define LEITOR_ARQUIVOS_HPP

#include <string>
#include <memory>
#include <vector>
#include <fstream> // Para std::ifstream
#include "entidades/RedeArmazens.hpp"
#include "entidades/SistemaTransporte.hpp"
#include "entidades/Pacote.hpp"
#include "utils/Tipos.hpp" // Para ParametrosSimulacaoGlobal

namespace LogisticSystem {

    class LeitorArquivos {
    public:
        // Função principal para ler a configuração global e a topologia
        // Retorna os parâmetros globais lidos.
        static ParametrosSimulacaoGlobal lerConfiguracaoESetup(
            const std::string& nomeArquivo,
            std::shared_ptr<RedeArmazens> redeArmazens,
            std::shared_ptr<SistemaTransporte> sistemaTransporte);

        // Função para ler apenas a seção de pacotes do arquivo
        // Retorna o vetor de shared_ptr para Pacotes e o número de pacotes lidos.
        static std::vector<std::shared_ptr<Pacote>> lerPacotes(
            const std::string& nomeArquivo,
            int& numeroPacotesOut);

    private:
        // Funções auxiliares para parsear diferentes partes do arquivo de entrada
        static std::vector<std::string> splitString(const std::string& s, char delimiter);

        // Auxiliar para ler os 4 parâmetros globais iniciais
        static void lerParametrosGlobais(std::ifstream& arquivo, ParametrosSimulacaoGlobal& parametros);

        // Auxiliar para ler o número de armazéns
        static int lerNumeroArmazens(std::ifstream& arquivo);

        // Auxiliar para configurar armazéns e rotas com base na matriz de adjacência
        static void configurarArmazensERotas(
            std::ifstream& arquivo, int numArmazens,
            std::shared_ptr<RedeArmazens> redeArmazens,
            std::shared_ptr<SistemaTransporte> sistemaTransporte,
            const ParametrosSimulacaoGlobal& parametros);

        // Auxiliar para pular o cabeçalho e a matriz de adjacência
        static void pularSecaoConfiguracao(std::ifstream& arquivo); // Apenas pular a secao de config

        // Auxiliar para ler o número de pacotes
        static int lerNumeroPacotes(std::ifstream& arquivo);

        // Auxiliar para ler os dados de cada pacote
        static std::vector<std::shared_ptr<Pacote>> lerDadosPacotes(
            std::ifstream& arquivo, int numeroPacotes);
    };

} // namespace LogisticSystem

#endif // LEITOR_ARQUIVOS_HPP
