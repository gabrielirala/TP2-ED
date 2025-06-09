#include "utils/LeitorArquivos.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <algorithm> // Para std::remove_if, std::isspace
#include <limits>    // Para std::numeric_limits

namespace LogisticSystem {

    // Função auxiliar para dividir strings (já existente, mantida)
    std::vector<std::string> LeitorArquivos::splitString(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    // Auxiliar para ler os 4 parâmetros globais iniciais
    void LeitorArquivos::lerParametrosGlobais(std::ifstream& arquivo, ParametrosSimulacaoGlobal& parametros) {
        std::string linha;

        if (std::getline(arquivo, linha)) {
            parametros.capacidadeTransporteGlobal = std::stoi(linha);
        } else { throw std::runtime_error("Erro de leitura: capacidadeTransporteGlobal nao encontrada."); }

        if (std::getline(arquivo, linha)) {
            parametros.latenciaTransporteGlobal = std::stod(linha);
        } else { throw std::runtime_error("Erro de leitura: latenciaTransporteGlobal nao encontrada."); }

        if (std::getline(arquivo, linha)) {
            parametros.intervaloTransportesGlobal = std::stod(linha);
        } else { throw std::runtime_error("Erro de leitura: intervaloTransportesGlobal nao encontrada."); }

        if (std::getline(arquivo, linha)) {
            parametros.custoRemocaoGlobal = std::stod(linha);
        } else { throw std::runtime_error("Erro de leitura: custoRemocaoGlobal nao encontrada."); }
    }

    // Auxiliar para ler o número de armazéns
    int LeitorArquivos::lerNumeroArmazens(std::ifstream& arquivo) {
        std::string linha;
        if (std::getline(arquivo, linha)) {
            return std::stoi(linha);
        }
        throw std::runtime_error("Erro de leitura: numeroarmazens nao encontrado.");
    }

    // Auxiliar para configurar armazéns e rotas com base na matriz de adjacência
    void LeitorArquivos::configurarArmazensERotas(
        std::ifstream& arquivo, int numArmazens,
        std::shared_ptr<RedeArmazens> redeArmazens,
        std::shared_ptr<SistemaTransporte> sistemaTransporte,
        const ParametrosSimulacaoGlobal& parametros) {

        // Criar Armazéns (IDs de 0 a numArmazens-1)
        for (ID_t i = 0; i < numArmazens; ++i) {
            // Capacidade total do armazém e capacidade da seção não são especificadas no novo enunciado.
            // Usaremos capacidade infinita (max_int) para o armazém e seções.
            std::string nomePadrao = "Armazem_" + std::to_string(i);
            redeArmazens->adicionarArmazem(i, nomePadrao, std::numeric_limits<Capacity_t>::max());
        }

        // Ler a matriz de adjacência e configurar rotas e seções
        for (ID_t i = 0; i < numArmazens; ++i) {
            std::string linhaMatriz;
            if (std::getline(arquivo, linhaMatriz)) {
                // Remover espaços em branco, se houver, antes de processar a linha da matriz
                linhaMatriz.erase(std::remove_if(linhaMatriz.begin(), linhaMatriz.end(), ::isspace), linhaMatriz.end());

                if (linhaMatriz.length() != static_cast<size_t>(numArmazens)) {
                    throw std::runtime_error("Erro de leitura: linha " + std::to_string(i) + " da matriz de adjacencia tem tamanho incorreto.");
                }

                for (ID_t j = 0; j < numArmazens; ++j) {
                    if (linhaMatriz[j] == '1') {
                        // Adicionar aresta no Grafo (RedeArmazens já faz isso internamente)
                        // Não precisamos chamar redeArmazens->adicionarAresta(i, j); aqui,
                        // pois a lógica de criação de rota em SistemaTransporte já adiciona a aresta ao Grafo.

                        // Configurar a seção no armazém de origem para o destino
                        // Adicionar seção apenas se ainda não existir (para evitar duplicação em adjacências bidirecionais)
                        if (!redeArmazens->obterArmazem(i)->temSecao(j)) {
                            redeArmazens->obterArmazem(i)->adicionarSecao(
                                j, std::numeric_limits<Capacity_t>::max(), parametros.custoRemocaoGlobal);
                        }

                        // Adicionar rota de transporte (se não for para o mesmo armazém e se ainda não existir)
                        // A topologia é bidirecional, então só precisamos adicionar uma vez para cada par (i, j)
                        // ou garantir que sistemaTransporte::adicionarRota lida com a bidirecionalidade ou evita duplicação
                        if (i != j && !sistemaTransporte->existeRota(i, j) && !sistemaTransporte->existeRota(j, i)) {
                             sistemaTransporte->adicionarRota(
                                i, j, parametros.latenciaTransporteGlobal, parametros.capacidadeTransporteGlobal);
                             // Como a topologia é bidirecional, também adicionamos a rota inversa
                             sistemaTransporte->adicionarRota(
                                j, i, parametros.latenciaTransporteGlobal, parametros.capacidadeTransporteGlobal);
                        }
                    }
                }
            } else {
                throw std::runtime_error("Erro de leitura: matriz de adjacencia incompleta ou linha ausente para armazem " + std::to_string(i) + ".");
            }
        }
    }

    // Função principal para ler a configuração global e a topologia
    ParametrosSimulacaoGlobal LeitorArquivos::lerConfiguracaoESetup(
        const std::string& nomeArquivo,
        std::shared_ptr<RedeArmazens> redeArmazens,
        std::shared_ptr<SistemaTransporte> sistemaTransporte) {

        std::ifstream arquivo(nomeArquivo);
        if (!arquivo.is_open()) { // Corrigido de 'file' para 'arquivo'
            throw std::runtime_error("Nao foi possivel abrir o arquivo de entrada: " + nomeArquivo);
        }

        ParametrosSimulacaoGlobal parametros;

        lerParametrosGlobais(arquivo, parametros); // Lê os 4 parâmetros iniciais

        int numArmazens = lerNumeroArmazens(arquivo); // Lê o número de armazéns

        // Configura armazéns e rotas baseado na matriz de adjacência
        configurarArmazensERotas(arquivo, numArmazens, redeArmazens, sistemaTransporte, parametros);

        // Nota: O arquivo ifstream 'arquivo' está agora posicionado após a matriz de adjacência.
        // A função lerPacotes precisará reabrir o arquivo e pular o que já foi lido
        // para encontrar a seção de pacotes, ou este método deveria retornar o ifstream.
        // Para a simplicidade, vamos assumir que lerPacotes reabre e avança.

        return parametros;
    }

    // Auxiliar para pular o cabeçalho e a matriz de adjacência na função lerPacotes
    void LeitorArquivos::pularSecaoConfiguracao(std::ifstream& arquivo) {
        std::string linha;
        // Pular 4 linhas de parâmetros globais
        for (int i = 0; i < 4; ++i) {
            if (!std::getline(arquivo, linha)) {
                throw std::runtime_error("Erro: arquivo de entrada muito curto, faltando parametros iniciais.");
            }
        }

        // Pular a linha do numeroarmazens e ler o seu valor
        if (!std::getline(arquivo, linha)) {
            throw std::runtime_error("Erro: arquivo de entrada muito curto, faltando numeroarmazens.");
        }
        int numArmazensParaPular = std::stoi(linha);

        // Pular as linhas da matriz de adjacência
        for (int i = 0; i < numArmazensParaPular; ++i) {
            if (!std::getline(arquivo, linha)) {
                throw std::runtime_error("Erro: arquivo de entrada muito curto, faltando linhas da matriz de adjacencia.");
            }
        }
    }

    // Auxiliar para ler o número de pacotes
    int LeitorArquivos::lerNumeroPacotes(std::ifstream& arquivo) {
        std::string linha;
        if (std::getline(arquivo, linha)) {
            return std::stoi(linha);
        }
        throw std::runtime_error("Erro de leitura: numeropacotes nao encontrado na secao de pacotes.");
    }

    // Auxiliar para ler os dados de cada pacote
    std::vector<std::shared_ptr<Pacote>> LeitorArquivos::lerDadosPacotes(
        std::ifstream& arquivo, int numeroPacotes) {
        
        std::vector<std::shared_ptr<Pacote>> pacotes;
        for (int i = 0; i < numeroPacotes; ++i) {
            std::string linha;
            if (std::getline(arquivo, linha)) {
                std::istringstream iss(linha);
                Timestamp_t tempoChegada;
                ID_t idPacote, armazemOrigem, armazemDestino;
                std::string dummy_pac, dummy_org, dummy_dst; // Para strings "pac", "org", "dst"

                // Exemplo: "1.0 pac 0 org 1 dst 3"
                iss >> tempoChegada >> dummy_pac >> idPacote >> dummy_org >> armazemOrigem >> dummy_dst >> armazemDestino;

                // Criar e adicionar o pacote. Assumindo que o construtor Pacote agora aceita esses 4 parâmetros.
                pacotes.push_back(std::make_shared<Pacote>(idPacote, tempoChegada, armazemOrigem, armazemDestino));
            } else {
                throw std::runtime_error("Erro de leitura: pacotes incompletos ou faltando. Esperado " + std::to_string(numeroPacotes) + " pacotes.");
            }
        }
        return pacotes;
    }

    // Função para ler a seção de pacotes do arquivo de entrada principal
    // (Esta é a sobrecarga que será chamada pelo Simulador)
    std::vector<std::shared_ptr<Pacote>> LeitorArquivos::lerPacotes(
        const std::string& nomeArquivo, int& numeroPacotesOut) {

        std::ifstream arquivo(nomeArquivo);
        if (!arquivo.is_open()) {
            throw std::runtime_error("Nao foi possivel reabrir o arquivo de entrada para pacotes: " + nomeArquivo);
        }

        pularSecaoConfiguracao(arquivo); // Pula a parte de configuração já lida

        numeroPacotesOut = lerNumeroPacotes(arquivo); // Lê o número de pacotes

        return lerDadosPacotes(arquivo, numeroPacotesOut); // Lê os dados dos pacotes
    }

} // namespace LogisticSystem