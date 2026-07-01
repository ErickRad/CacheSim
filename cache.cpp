#include "cache.hpp"

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <iomanip>

CacheSimulator::CacheSimulator(const CacheConfig& config) : config_(config), global_counter_(0){

    if (config_.nsets <= 0 || (config_.nsets & (config_.nsets - 1)) != 0)
        throw std::invalid_argument("nsets deve ser potência de 2 e > 0");

    if (config_.bsize <= 0 || (config_.bsize & (config_.bsize - 1)) != 0)
        throw std::invalid_argument("bsize deve ser potência de 2 e > 0");

    if (config_.assoc <= 0)
        throw std::invalid_argument("assoc deve ser > 0");

    offset_bits_ = log2i(config_.bsize);
    index_bits_  = log2i(config_.nsets);
    tag_bits_    = 32 - offset_bits_ - index_bits_;

    if (tag_bits_ < 0)
        throw std::invalid_argument("Configuração inválida: tag_bits negativo");

    sets_.assign(config_.nsets, std::vector<CacheLine>(config_.assoc));

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

int CacheSimulator::log2i(int value){
    int bits = 0;

    while (value > 1) {
        value >>= 1;
        bits++;
    }

    return bits;
}

void CacheSimulator::decodeAddress(uint32_t address, uint32_t& tag, uint32_t& index) const {
    uint32_t index_mask  = (1u << index_bits_)  - 1u;

    index = (address >> offset_bits_) & index_mask;
    tag   = address >> (offset_bits_ + index_bits_);
}

int CacheSimulator::selectVictim(int set_index){
    auto& set = sets_[set_index];

    switch (config_.policy) {

        case ReplacementPolicy::RANDOM: {
            return std::rand() % config_.assoc;
        }

        case ReplacementPolicy::FIFO: {
            int oldest_way = 0;
            uint64_t oldest_val = set[0].fifo_count;

            for (int w = 1; w < config_.assoc; w++) {

                if (set[w].fifo_count < oldest_val) {
                    oldest_val = set[w].fifo_count;
                    oldest_way = w;
                }
            }

            return oldest_way;
        }

        case ReplacementPolicy::LRU: {
            int lru_way = 0;
            uint64_t lru_val = set[0].lru_count;

            for (int w = 1; w < config_.assoc; w++) {

                if (set[w].lru_count < lru_val) {
                    lru_val = set[w].lru_count;
                    lru_way = w;
                }
            }

            return lru_way;
        }
    }

    return 0;
}

void CacheSimulator::run(){

    std::ifstream fin(config_.input_file, std::ios::binary);

    if (!fin.is_open())
        throw std::runtime_error("Não foi possível abrir: " + config_.input_file);

    int fa_capacity = config_.nsets * config_.assoc;

    std::list<uint64_t> lru_list;
    std::unordered_map<uint64_t, std::list<uint64_t>::iterator> lru_map;
    std::unordered_set<uint64_t> seen_blocks;

    uint32_t raw_address = 0;

    while (fin.read(reinterpret_cast<char*>(&raw_address), sizeof(raw_address))) {
        
        
        uint32_t address = ((raw_address & 0xFF000000u) >> 24) |
                           ((raw_address & 0x00FF0000u) >>  8) |
                           ((raw_address & 0x0000FF00u) <<  8) |
                           ((raw_address & 0x000000FFu) << 24);


        global_counter_++;
        stats_.total_accesses++;

        uint32_t tag = 0, index = 0;
       
        decodeAddress(address, tag, index);
        uint64_t block_id = static_cast<uint64_t>(address) >> offset_bits_;
        
        auto& real_set = sets_[index];

        bool real_hit = false;

        for (int w = 0; w < config_.assoc; w++) {

            if (real_set[w].valid && real_set[w].tag == tag) {
                real_hit = true;
                real_set[w].lru_count = global_counter_;

                break;
            }
        }

        bool first_access = (seen_blocks.find(block_id) == seen_blocks.end());
        
        seen_blocks.insert(block_id);

        bool fa_hit = (lru_map.find(block_id) != lru_map.end());

        if (fa_hit) {
            lru_list.erase(lru_map[block_id]);

        } else if (static_cast<int>(lru_list.size()) >= fa_capacity) {
            uint32_t evicted = lru_list.back();
            lru_list.pop_back();
            lru_map.erase(evicted);
        }

        lru_list.push_front(block_id);
        lru_map[block_id] = lru_list.begin();

        if (real_hit) {
            stats_.hits++;
            continue;
        }

        stats_.misses++;

        if (first_access) {
            stats_.compulsory_misses++;

        } else if (!fa_hit) {
            stats_.capacity_misses++;

        } else {
            stats_.conflict_misses++;
        }

        int target_way = -1;

        for (int w = 0; w < config_.assoc; w++) {

            if (!real_set[w].valid) {
                target_way = w;
                break;
            }
        }

        if (target_way == -1) target_way = selectVictim(index);

        real_set[target_way].valid      = true;
        real_set[target_way].tag        = tag;
        real_set[target_way].fifo_count = global_counter_;
        real_set[target_way].lru_count  = global_counter_;
    }

    fin.close();
}

void CacheSimulator::printStats() const{
    const auto& s = stats_;

    double hit_rate = (s.total_accesses > 0)
        ? static_cast<double>(s.hits) / s.total_accesses : 0.0;

    double miss_rate = 1.0 - hit_rate;

    double comp_rate = (s.misses > 0)
        ? static_cast<double>(s.compulsory_misses) / s.misses : 0.0;

    double cap_rate = (s.misses > 0)
        ? static_cast<double>(s.capacity_misses)  / s.misses : 0.0;

    double conf_rate = (s.misses > 0)
        ? static_cast<double>(s.conflict_misses)  / s.misses : 0.0;

    if (config_.flag == 0) {
        std::cout << "==============================\n";
        std::cout << " RELATÓRIO DO SIMULADOR DE CACHE\n";
        std::cout << "==============================\n";
        std::cout << "Configuração:\n";
        std::cout << "  Conjuntos (nsets)   : " << config_.nsets  << "\n";
        std::cout << "  Tamanho bloco (B)   : " << config_.bsize  << " bytes\n";
        std::cout << "  Associatividade     : " << config_.assoc  << "-way\n";

        std::string pol_str;
        switch (config_.policy) {
            case ReplacementPolicy::RANDOM: pol_str = "Random";  break;
            case ReplacementPolicy::FIFO:   pol_str = "FIFO";    break;
            case ReplacementPolicy::LRU:    pol_str = "LRU";     break;
        }

        std::cout << "  Política subst.     : " << pol_str << "\n";
        std::cout << "  Capacidade total    : "
                  << config_.nsets * config_.bsize * config_.assoc << " bytes\n";

        std::cout << "------------------------------\n";
        std::cout << "Resultados:\n";
        std::cout << "  Total de acessos    : " << s.total_accesses << "\n";
        std::cout << "  Hits                : " << s.hits           << "\n";
        std::cout << "  Misses              : " << s.misses         << "\n";
        std::cout << "  Misses compulsórios : " << s.compulsory_misses << "\n";
        std::cout << "  Misses de capacidade: " << s.capacity_misses   << "\n";
        std::cout << "  Misses de conflito  : " << s.conflict_misses   << "\n";

        std::cout << "------------------------------\n";
        std::cout << "Taxas:\n";
        std::cout << "  Taxa de hit         : " << hit_rate  * 100.0 << " %\n";
        std::cout << "  Taxa de miss        : " << miss_rate * 100.0 << " %\n";
        std::cout << std::fixed << std::setprecision(2)  << "  Taxa miss compuls.  : " << comp_rate * 100.0 << " %\n";
        std::cout << std::fixed << std::setprecision(2)  << "  Taxa miss capac.    : " << cap_rate  * 100.0 << " %\n";
        std::cout << std::fixed << std::setprecision(2)  << "  Taxa miss conflito  : " << conf_rate * 100.0 << " %\n";

        std::cout << "==============================\n";

    } else {

        std::cout << std::fixed;
        std::cout.precision(4);
        std::cout << s.total_accesses << " "
                  << hit_rate  << " "
                  << miss_rate << " "
                  << comp_rate << " "
                  << cap_rate  << " "
                  << conf_rate << "\n";
    }
}