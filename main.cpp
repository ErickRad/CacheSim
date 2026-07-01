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
 *   ./simulate 256 4 1 R 1 bin_100.bin
 */

#include <iostream>
#include <stdexcept>
#include <string>
#include <cstdlib>

#include "cache.hpp"

static void usage(const char* prog_name){

    std::cerr << "Uso: " << prog_name
              << " <nsets> <bsize> <assoc> <subst> <flag> <file>\n"
              << "\n"
              << "  nsets      - número de conjuntos (potência de 2)\n"
              << "  bsize      - tamanho do bloco em bytes (potência de 2)\n"
              << "  assoc      - grau de associatividade\n"
              << "  subst      - política de substituição: random | fifo | lru\n"
              << "  flag       - 0 (verbose) | 1 (restrita numérica)\n"
              << "  file       - arquivo binário de endereços\n"
              << "\n"
              << "Exemplo:\n"
              << "  simulate 256 4 1 random 1 bin_100.bin\n";
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
        config.flag        = std::stoi(argv[5]);
        config.input_file  = argv[6];

    } catch (const std::exception& e) {
        std::cerr << "Erro ao parsear argumentos: " << e.what() << "\n";
        usage(argv[0]);

        return EXIT_FAILURE;
    }

    std::string policy_str = argv[4];

    if (policy_str == "random" || policy_str == "R") {
        config.policy = ReplacementPolicy::RANDOM;

    } else if (policy_str == "fifo" || policy_str == "F") {
        config.policy = ReplacementPolicy::FIFO;

    } else if (policy_str == "lru" || policy_str == "L") {
        config.policy = ReplacementPolicy::LRU;

    } else {
        std::cerr << "Política inválida: '" << policy_str
                  << "'. Use random, fifo ou lru.\n";

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
