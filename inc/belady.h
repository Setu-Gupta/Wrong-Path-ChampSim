#ifndef BELADY_H
#define BELADY_H

#include <cstdint>
#include <cstdlib>
#include <map>
#include <string>
#include <utility>
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

                // The key is the set number and the value is an ordered list of cache accesses
                // (both demands and prefetches). Cache accesses are represented by a pair of values
                // <address, cycle> which cycles is event cycle at which the cache was accessed
                std::map<uint64_t, std::vector<std::pair<uint64_t, uint64_t>>> raw_accesses;

                // The key is the set number and the value is an ordered list of cache accesses
                // (both demands and prefetches)
                std::map<uint64_t, std::vector<uint64_t>> sorted_accesses;

                // The key is the set number and the value is the first unread index in the access
                // list. This is only used for replaying
                std::map<uint64_t, decltype(sorted_accesses)::size_type> indices;
                
                public:
                        void initialize(const std::string& NAME, const uint32_t NUM_SET);
                        void finalize();
                        void cache_access(const uint32_t set, const uint64_t full_addr, const uint64_t event_cycle);
                        uint32_t find_victim(const uint32_t set, const std::vector<uint64_t>& set_contents, const uint64_t full_addr);
        };
}

#endif
