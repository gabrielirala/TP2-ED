#include <iostream>
#include <stdexcept>
#include "Simulation.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Erro: Nenhum arquivo de entrada especificado." << std::endl;
        std::cerr << "Uso: ./bin/tp2.out <arquivo_de_entrada>" << std::endl;
        return 1;
    }

    try {
        Simulation sim(argv[1]);
        sim.run();
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}