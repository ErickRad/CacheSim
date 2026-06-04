#ifndef CACHE_H
#define CACHE_H

/**
 * @file cache.h
 * @brief Definições e estruturas do simulador de cache
 *
 * Este arquivo contém todas as estruturas de dados e declarações
 * necessárias para o simulador de cache parametrizável.
 *
 * Suporta:
 *  - Cache diretamente mapeada (assoc = 1)
 *  - Cache totalmente associativa (nsets = 1)
 *  - Cache set-associativa (assoc > 1, nsets > 1)
 *
 * Políticas de substituição: Random (R), FIFO (F), LRU (L)
 * Endereçamento: 32 bits, byte-addressable, big-endian
 */

#include <cstdint>
#include <vector>
#include <string>

// ─────────────────────────────────────────────
// Política de substituição
// ─────────────────────────────────────────────
enum class ReplacementPolicy {
    RANDOM,
    FIFO,
    LRU
};

// ─────────────────────────────────────────────
// Uma linha (bloco) dentro de um conjunto
// ─────────────────────────────────────────────
struct CacheLine {
    bool      valid;      ///< Indica se o bloco é válido
    uint32_t  tag;        ///< Tag do bloco armazenado
    uint64_t  fifo_order; ///< Contador de inserção (para FIFO)
    uint64_t  lru_order;  ///< Contador de último uso (para LRU)

    CacheLine() : valid(false), tag(0), fifo_order(0), lru_order(0) {}
};

// ─────────────────────────────────────────────
// Estatísticas de simulação
// ─────────────────────────────────────────────
struct CacheStats {
    uint64_t total_accesses   = 0; ///< Total de acessos
    uint64_t hits             = 0; ///< Acertos
    uint64_t misses           = 0; ///< Total de misses
    uint64_t compulsory_misses = 0; ///< Misses compulsórios (cold miss)
    uint64_t capacity_misses  = 0; ///< Misses de capacidade
    uint64_t conflict_misses  = 0; ///< Misses de conflito
};

// ─────────────────────────────────────────────
// Configuração da cache
// ─────────────────────────────────────────────
struct CacheConfig {
    int               nsets;        ///< Número de conjuntos
    int               bsize;        ///< Tamanho do bloco em bytes
    int               assoc;        ///< Grau de associatividade
    ReplacementPolicy policy;       ///< Política de substituição
    int               flag_output;  ///< 0 = verbose, 1 = padrão
    std::string       input_file;   ///< Arquivo de entrada binário
};

// ─────────────────────────────────────────────
// Classe principal do simulador de cache
// ─────────────────────────────────────────────
class CacheSimulator {
public:
    /**
     * @brief Construtor — inicializa a cache com a configuração fornecida
     * @param config Estrutura com todos os parâmetros da cache
     */
    explicit CacheSimulator(const CacheConfig& config);

    /**
     * @brief Executa a simulação lendo os endereços do arquivo binário
     */
    void run();

    /**
     * @brief Exibe o relatório final de estatísticas
     */
    void printStats() const;

private:
    CacheConfig                         config_;
    std::vector<std::vector<CacheLine>> sets_;   ///< sets_[conjunto][via]
    CacheStats                          stats_;

    // Contadores globais para ordenação FIFO/LRU
    uint64_t global_counter_;

    // Número de bits de offset, índice e tag
    int offset_bits_;
    int index_bits_;
    int tag_bits_;

    /**
     * @brief Acessa um endereço na cache (simula um acesso de leitura)
     * @param address Endereço de 32 bits
     */
    void access(uint32_t address);

    /**
     * @brief Decompõe um endereço nos campos: tag, índice e offset
     */
    void decodeAddress(uint32_t address, uint32_t& tag,
                       uint32_t& index, uint32_t& offset) const;

    /**
     * @brief Seleciona a via a ser substituída dentro de um conjunto
     * @param set_index Índice do conjunto
     * @return Índice da via escolhida para substituição
     */
    int selectVictim(int set_index);

    /**
     * @brief Calcula log2 inteiro (auxiliar)
     */
    static int log2i(int value);
};

#endif // CACHE_H
