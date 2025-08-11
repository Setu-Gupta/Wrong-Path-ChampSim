#include <cassert>
#include <iostream>
#include <memory>
#include <random>
#include "cache.h"

namespace random_rpl
{
        // Ref: https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution.html
        std::random_device dev;
        std::mt19937 rng = std::mt19937(random_rpl::dev());
        using dist_type = std::uniform_int_distribution<std::mt19937::result_type>;
        std::unique_ptr<dist_type> dist;
}

void CACHE::initialize_replacement()
{
         random_rpl::dist = std::make_unique<random_rpl::dist_type>(0, NUM_WAY);
}

uint32_t CACHE::find_victim(uint32_t triggering_cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
        assert(random_rpl::dist);
        return static_cast<uint32_t>((*random_rpl::dist)(random_rpl::rng));
}

void CACHE::update_replacement_state(uint32_t triggering_cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type,
                                     uint8_t hit) {}

void CACHE::replacement_final_stats() {}
