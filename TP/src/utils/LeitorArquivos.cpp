#include "utils/LeitorArquivos.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

namespace LogisticSystem {

    bool LeitorArquivos::lerTopologia(const std::string& arquivo,
                                     std::shared_ptr<RedeArmazens> redeArmazens,
                                     std::shared_ptr<SistemaTransporte> sistemaTransporte,
                                     const ConfiguracaoSistema& configSistema) {
        std::ifstream file(arquivo);
        if (!file.is_open()) {
            std::cerr << "Erro: Nao foi possivel abrir o arquivo de topologia: " << arquivo << std::endl;
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::vector<std::string> tokens = splitString(line, ';');
            if (tokens.empty()) continue;

            if (tokens[0] == "ARMAZEM" && tokens.size() == 3) {
                try {
                    ID_t id = std::stoi(tokens[1]);
                    std::string nome = tokens[2];
                    // A capacidade total do armazém é a soma das capacidades das seções.
                    // Ou pode ser um valor fixo definido na configuração padrão.
                    // Para simplificar, Armazem será criado com capacidade 0 e seções adicionadas depois.
                    redeArmazens->adicionarArmazem(id, nome, 0); // Capacidade será ajustada pelas secoes
                } catch (const std::exception& e) {
                    std::cerr << "Erro ao parsear linha ARMAZEM: " << line << " - " << e.what() << std::endl;
                    return false;
                }
            } else if (tokens[0] == "SECAO" && tokens.size() == 5) {
                try {
                    ID_t armazem_id = std::stoi(tokens[1]);
                    ID_t destino_secao_id = std::stoi(tokens[2]);
                    Capacity_t capacidade_secao = std::stoi(tokens[3]);
                    Distance_t tempo_manipulacao = std::stod(tokens[4]);

                    auto armazem_ptr = redeArmazens->obterArmazem(armazem_id);
                    if (armazem_ptr) {
                        armazem_ptr->adicionarSecao(destino_secao_id, capacidade_secao, tempo_manipulacao);
                        // Atualiza capacidade total do armazém
                        // armazem_ptr->capacidadeTotal += capacidade_secao; // Se fosse um membro publico
                    } else {
                        std::cerr << "Erro: Armazem " << armazem_id << " nao encontrado para adicionar secao." << std::endl;
                        return false;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Erro ao parsear linha SECAO: " << line << " - " << e.what() << std::endl;
                    return false;
                }
            } else if (tokens[0] == "ROTA" && tokens.size() == 5) {
                try {
                    ID_t origem_id = std::stoi(tokens[1]);
                    ID_t destino_id = std::stoi(tokens[2]);
                    Distance_t tempo_transporte = std::stod(tokens[3]);
                    Capacity_t capacidade_rota = std::stoi(tokens[4]);

                    sistemaTransporte->adicionarRota(origem_id, destino_id, tempo_transporte, capacidade_rota);
                } catch (const std::exception& e) {
                    std::cerr << "Erro ao parsear linha ROTA: " << line << " - " << e.what() << std::endl;
                    return false;
                }
            }
            // Ignorar outras linhas ou tratar como erro de formato
        }

        // Configurar parâmetros globais do sistema de transporte
        sistemaTransporte->configurarParametrosGlobais(
            configSistema.intervaloTransporte,
            configSistema.tempoTransportePadrao,
            configSistema.capacidadeTransportePadrao
        );

        return true;
    }

    std::vector<std::shared_ptr<Pacote>> LeitorArquivos::lerPacotes(const std::string& arquivo) {
        std::vector<std::shared_ptr<Pacote>> pacotes;
        std::ifstream file(arquivo);
        if (!file.is_open()) {
            std::cerr << "Erro: Nao foi possivel abrir o arquivo de pacotes: " << arquivo << std::endl;
            return pacotes;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::vector<std::string> tokens = splitString(line, ';');
            if (tokens.size() == 7) {
                try {
                    ID_t id = std::stoi(tokens[0]);
                    Timestamp_t postagem = std::stod(tokens[1]);
                    std::string remetente = tokens[2];
                    std::string destinatario = tokens[3];
                    std::string tipo = tokens[4];
                    ID_t origem = std::stoi(tokens[5]);
                    ID_t destino = std::stoi(tokens[6]);

                    auto pacote = std::make_shared<Pacote>(id, postagem, remetente,
                                                          destinatario, tipo, origem, destino);
                    pacotes.push_back(pacote);
                } catch (const std::exception& e) {
                    std::cerr << "Erro ao parsear linha de pacote: " << line << " - " << e.what() << std::endl;
                }
            }
        }
        return pacotes;
    }

    std::vector<std::string> LeitorArquivos::splitString(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

} // namespace LogisticSystem