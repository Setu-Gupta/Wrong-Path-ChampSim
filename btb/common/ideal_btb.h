/*
 * This file implements a basic Branch Target Buffer (BTB) structure.
 * It uses a set-associative BTB to predict the targets.
 */

#include "ooo_cpu.h"
#include "branch_info.h"
#include <map>

class IdealBTB {

private:
  
  struct ENTRY {
    uint64_t target;
    uint8_t branch_info;
  };
  
  std::map<uint64_t, ENTRY> btb;
  
public:
  
  void initialize() {
    std::cout << "Ideal BTB" << std::endl;
    btb.clear();
  }
    
  std::pair<uint64_t, uint8_t> predict(uint64_t ip) {
    if (btb.count(ip) == 0) {
      // no prediction for this IP
      return std::make_pair(0, BRANCH_INFO_ALWAYS_TAKEN);
    }
    return std::make_pair(btb[ip].target, btb[ip].branch_info);
  }

  void update(uint64_t ip, uint64_t branch_target, uint8_t taken, uint8_t branch_type) {
    if (btb.count(ip) == 0) {
      if ((branch_target == 0) || !taken) {
        return;
      }
      // no prediction for this entry so far, so allocate one
      btb[ip].branch_info = BRANCH_INFO_ALWAYS_TAKEN;
    }
    // update btb entry
    if (branch_target != 0) btb[ip].target = branch_target;
    if ((branch_type == BRANCH_INDIRECT) || (branch_type == BRANCH_INDIRECT_CALL)) {
      btb[ip].branch_info = BRANCH_INFO_INDIRECT;
    } else if (branch_type == BRANCH_RETURN) {
      btb[ip].branch_info = BRANCH_INFO_RETURN;
    } else if (!taken) {
      btb[ip].branch_info = BRANCH_INFO_CONDITIONAL;
    }
  }
};
