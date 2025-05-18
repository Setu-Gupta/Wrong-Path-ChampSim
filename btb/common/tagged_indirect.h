/*
 * This file implements a tagged (gshare like) associative indirect target branch prediction.
 */

#include "ooo_cpu.h"

template<uint32_t SETS, uint32_t WAYS>
class TaggedIndirect {

private:
  
  struct ENTRY {
    uint64_t ip_tag;
    uint64_t target;
    uint64_t lru;
  };

  ENTRY indirect[SETS][WAYS];
  uint64_t lru_counter;
  uint64_t conditional_history;

  uint64_t set_index(uint64_t ip) { return (((ip >> 2) ^ (conditional_history)) & (SETS - 1)); }

  ENTRY* find_entry(uint64_t ip) {
    uint64_t set = set_index(ip);
    for (uint32_t i = 0; i < WAYS; i++) {
      if (indirect[set][i].ip_tag == ip) {
        return &(indirect[set][i]);
      }
    }
    return NULL;
  }

  ENTRY* get_lru_entry(uint64_t set) {
    uint32_t lru_way = 0;
    uint64_t lru_value = indirect[set][lru_way].lru;
    for (uint32_t i = 0; i < WAYS; i++) {
      if (indirect[set][i].lru < lru_value) {
        lru_way = i;
        lru_value = indirect[set][lru_way].lru;
      }
    }
    return &(indirect[set][lru_way]);
  }
  
  void update_lru(ENTRY* indirect) {
    indirect->lru = lru_counter;
    lru_counter++;
  }

public:
  
  void initialize() {
    std::cout << "Indirect buffer sets: " << SETS << " ways: " << WAYS << std::endl;
    for (uint32_t i = 0; i < SETS; i++) {
      for (uint32_t j = 0; j < WAYS; j++) {
        indirect[i][j].ip_tag = 0;
        indirect[i][j].target = 0;
        indirect[i][j].lru = 0;
      }
    }
    lru_counter = 0;
    conditional_history = 0;
  }
  
  uint64_t predict(uint64_t ip) {
    auto ind_entry = find_entry(ip);
    if (ind_entry == NULL) {
      // no prediction for this IP
      return 0;
    }
    update_lru(ind_entry);
    return ind_entry->target;
  }

  void update(uint64_t ip, uint64_t branch_target, uint8_t taken, uint8_t branch_type) {
    if ((branch_type == BRANCH_INDIRECT) || (branch_type == BRANCH_INDIRECT_CALL)) {
      auto ind_entry = find_entry(ip);
      if (ind_entry == NULL) {
        // no prediction for this entry so far, so allocate one
        uint64_t set = set_index(ip);
        ind_entry = get_lru_entry(set);
        ind_entry->ip_tag = ip;
        update_lru(ind_entry);
      }
      ind_entry->target = branch_target;
    }
    if (branch_type == BRANCH_CONDITIONAL) {
      conditional_history <<= 1;
      if (taken) {
        conditional_history |= 1;
      }
    }
  }
};
