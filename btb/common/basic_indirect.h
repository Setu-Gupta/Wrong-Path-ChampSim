/*
 * This file implements a basic (gshare like) indirect target branch prediction.
 */

#include "ooo_cpu.h"

template<uint32_t INDIRECT_SIZE>
class BasicIndirect {

private:
  
  uint64_t indirect[INDIRECT_SIZE];
  uint64_t conditional_history;

  uint64_t indirect_hash(uint64_t ip) { return (((ip >> 2) ^ (conditional_history)) & (INDIRECT_SIZE - 1)); }

public:
  
  void initialize() {
    std::cout << "Indirect buffer size: " << INDIRECT_SIZE << std::endl;
    for (uint32_t i = 0; i < INDIRECT_SIZE; i++) {
      indirect[i] = 0;
    }
    conditional_history = 0;
  }
  
  uint64_t predict(uint64_t ip) { return indirect[indirect_hash(ip)]; }

  void update(uint64_t ip, uint64_t branch_target, uint8_t taken, uint8_t branch_type) {
    if ((branch_type == BRANCH_INDIRECT) || (branch_type == BRANCH_INDIRECT_CALL)) {
      indirect[indirect_hash(ip)] = branch_target;
    }
    if (branch_type == BRANCH_CONDITIONAL) {
      conditional_history <<= 1;
      if (taken) {
        conditional_history |= 1;
      }
    }
  }
};
