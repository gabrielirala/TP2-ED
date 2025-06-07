#include "core/Simulador.hpp"
#include "utils/LeitorArquivos.hpp"
#include <iostream>
#include <exception>

using namespace LogisticSystem;

int main(int argc, char* argv[]) {
    try {
        // Configuração inicial
        ParametrosSimulacao params;
        
        // Parsing de argumentos da linha de comando
        if (argc > 1) {
            for (int i = 1; i < argc; i++) {
                std::string arg = argv[i];
                if (arg == "--debug") {
                    params.modoDebug = true;
                } else if (arg == "--config" && i + 1 < argc) {
                    // Carregar arquivo de configuração personalizado
                } else if (arg == "--topologia" && i + 1 < argc) {
                    params.arquivoTopologia = argv[++i];
                } else if (arg == "--pacotes" && i + 1 < argc) {
                    params.arquivoPacotes = argv[++i];
                } else if (arg == "--saida" && i + 1 < argc) {
                    params.arquivoSaida = argv[++i];
                }
            }
        } else {
            // Configuração padrão
            params.arquivoTopologia = "dados/topologia.txt";
            params.arquivoPacotes = "dados/entrada.txt";
            params.arquivoSaida = "resultados/relatorio.txt";
        }
        
        // Criar e configurar simulador
        Simulador simulador;
        simulador.carregarParametros(params);
        
        std::cout << "Inicializando simulação..." << std::endl;
        if (!simulador.inicializar()) {
            std::cerr << "Erro na inicialização do simulador" << std::endl;
            return 1;
        }
        
        std::cout << "Executando simulação..." << std::endl;
        if (!simulador.executarSimulacao()) {
            std::cerr << "Erro durante a execução da simulação" << std::endl;
            return 1;
        }
        
        std::cout << "Gerando relatórios..." << std::endl;
        simulador.gerarRelatorios();
        simulador.salvarEstatisticas();
        
        std::cout << "Simulação concluída com sucesso!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Erro fatal: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Erro desconhecido" << std::endl;
        return 1;
    }
    
    return 0;
}
