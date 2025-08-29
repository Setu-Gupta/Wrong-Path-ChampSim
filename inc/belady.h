#ifndef BELADY_H
#define BELADY_H

#include <cstdint>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>

namespace Belady
{
        class BeladyReplacementPolicy
        {
                enum class State
                {
                        unknown,
                        trace,          // Trace the cache accesses and dump them to the trace file
                        replay          // Read the trace file and use it for cache replacement
                };

                std::string trace_file;
                State state = State::unknown;
                std::map<uint64_t, std::vector<uint64_t>> accesses;     // The key is the set number and the value is an ordered list of cache accesses (both demands and prefetches)
                std::map<uint64_t, std::vector<uint64_t>::size_type> indices;   // The key is the set number and the value is the first unread index in the access list. This is only used for replaying
                
                public:
                        void initialize(const std::string& NAME, const uint32_t NUM_SET);
                        void finalize() const;
                        void cache_access(const uint32_t set, const uint64_t full_addr, const uint64_t event_cycle);
                        uint32_t find_victim(const uint32_t set, const std::vector<uint64_t>& set_contents, const uint64_t full_addr);
        };
}

#endif
