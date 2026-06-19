/**
 * @file main.cpp
 * @brief Ponto de entrada do simulador de cache
 *
 * Uso:
 *   simulate <nsets> <bsize> <assoc> <substituição> <flag_saida> <arquivo>
 *
 * Parâmetros:
 *   nsets        — número de conjuntos (potência de 2)
 *   bsize        — tamanho do bloco em bytes (potência de 2)
 *   assoc        — grau de associatividade (≥ 1)
 *   substituição — política: R (Random), F (FIFO), L (LRU)
 *   flag_saida   — 0 = saída verbose; 1 = saída restrita (numérica)
 *   arquivo      — arquivo binário de endereços (big-endian, 32 bits cada)
 *
 * Exemplo:
 *   simulate 256 4 1 R 1 bin_100.bin
 */

#include <iostream>
#include <stdexcept>
#include <string>
#include <cstdlib>

#include "cache.h"

static void usage(const char* prog_name){

    std::cerr << "Uso: " << prog_name
              << " <nsets> <bsize> <assoc> <subst> <flag_saida> <arquivo>\n"
              << "\n"
              << "  nsets      - número de conjuntos (potência de 2)\n"
              << "  bsize      - tamanho do bloco em bytes (potência de 2)\n"
              << "  assoc      - grau de associatividade\n"
              << "  subst      - política de substituição: R | F | L\n"
              << "  flag_saida - 0 (verbose) | 1 (restrita numérica)\n"
              << "  arquivo    - arquivo binário de endereços\n"
              << "\n"
              << "Exemplo:\n"
              << "  cache_simulator 256 4 1 R 1 bin_100.bin\n";
}

int main(int argc, char* argv[])
{
    if (argc != 7) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    CacheConfig config;

    try {
        config.nsets       = std::stoi(argv[1]);
        config.bsize       = std::stoi(argv[2]);
        config.assoc       = std::stoi(argv[3]);
        config.flag_output = std::stoi(argv[5]);
        config.input_file  = argv[6];

    } catch (const std::exception& e) {
        std::cerr << "Erro ao parsear argumentos: " << e.what() << "\n";
        usage(argv[0]);

        return EXIT_FAILURE;
    }

    std::string policy_str = argv[4];
    if (policy_str == "R" || policy_str == "r") {
        config.policy = ReplacementPolicy::RANDOM;

    } else if (policy_str == "F" || policy_str == "f") {
        config.policy = ReplacementPolicy::FIFO;

    } else if (policy_str == "L" || policy_str == "l") {
        config.policy = ReplacementPolicy::LRU;

    } else {
        std::cerr << "Política inválida: '" << policy_str
                  << "'. Use R, F ou L.\n";

        usage(argv[0]);
        return EXIT_FAILURE;
    }

    try {
        CacheSimulator sim(config);
        sim.run();
        sim.printStats();
        
    } catch (const std::exception& e) {
        std::cerr << "Erro durante a simulação: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
