#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <fmt/core.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <map>
#include <utility>
#include <vector>

#include "belady.h"

namespace Belady
{
        // Ref: https://stackoverflow.com/questions/631664/accessing-environment-variables-in-c
        inline std::string get_variable(const std::string& var)
        {
                const char* val = std::getenv(var.c_str());
                return val == NULL ? std::string("") : std::string(val);
        }
}

// Ref: https://stackoverflow.com/questions/79746016/serializing-and-compressing-stdmap-using-boost
void Belady::BeladyReplacementPolicy::initialize(const std::string& NAME, const uint32_t NUM_SET)
{
        // Figure out the current cache level
        std::string cache_level = "";
        if(NAME.find("L1D") != std::string::npos)
                cache_level = "L1D";
        else if(NAME.find("L1I") != std::string::npos)
                cache_level = "L1I";
        else if(NAME.find("L2C") != std::string::npos)
                cache_level = "L2C";
        else if(NAME.find("LLC") != std::string::npos)
                cache_level = "LLC";
        assert(cache_level != "");

        // Read the relevant environment variables
        const std::string trace_env_var = Belady::get_variable(cache_level + "_BELADY_TRACE");
        const std::string replay_env_var = Belady::get_variable(cache_level + "_BELADY_REPLAY");
        assert((trace_env_var == "") != (replay_env_var == ""));    // Either tracing or replaying should not be enabled, not both

        // Populate the member variables
        trace_file = (trace_env_var == "") ? replay_env_var : trace_env_var;
        state = (trace_env_var == "") ? State::replay : State::trace;
        fmt::println("{} is running Bélady in {} mode", cache_level, (state == State::replay) ? "replay" : "tracing");

        // Read the access trace from the trace file
        if(state == State::replay)
        {
                std::ifstream f(trace_file, std::ios::binary);
                if(f.fail())
                        fmt::println(stderr, "Couldn't open the {} archive for replaying", trace_file);
                boost::iostreams::filtering_istream filter;
                filter.push(boost::iostreams::gzip_decompressor());
                filter.push(f);

                boost::archive::binary_iarchive archive(filter);
                archive >> accesses;
                assert(accesses.size() == NUM_SET);

                // Initialize the indices
                for(uint32_t idx = 0; idx < NUM_SET; idx++) indices[idx] = 0;
                assert(indices.size() == accesses.size());
        }

        assert(state != State::unknown);
}

void Belady::BeladyReplacementPolicy::finalize()
{
        // Sort the accesses based on the event cycle
        for(auto kv: accesses)
        {
                // using access_type = std::pair<uint64_t, uint64_t>;
                std::sort(kv.second.begin(), kv.second.end(), [](const auto& lhs, const auto& rhs)
                                {return lhs.second < rhs.second;});
        }

        // Save the access trace to the trace file
        if(state == State::trace)
        {
                std::ofstream f(trace_file, std::ios::binary);
                if(f.fail())
                        fmt::println(stderr, "Couldn't create the {} archive", trace_file);
                boost::iostreams::filtering_ostream filter;
                filter.push(boost::iostreams::gzip_compressor());
                filter.push(f);
                boost::archive::binary_oarchive archive(filter);

                archive << accesses;
        }
}

void Belady::BeladyReplacementPolicy::cache_access(const uint32_t set, const uint64_t full_addr, const uint64_t event_cycle)
{
        if(state == State::trace)
                accesses[set].push_back({full_addr, event_cycle});
        else
        {
                // Verify that the trace matches current execution
                auto it = std::find(accesses[set].cbegin(), accesses[set].cend(), std::make_pair(full_addr, event_cycle));
                assert(it != accesses[set].cend());
        }
}

uint32_t Belady::BeladyReplacementPolicy::find_victim(const uint32_t set, const std::vector<uint64_t>& set_contents, const uint64_t fill_addr, const uint64_t cycle)
{
        // Find the new start position to search based on the current cycle
        const auto& trace = accesses[set];
        auto start_pos = trace.cbegin();
        std::advance(start_pos, indices[set]);
        const auto& search_start_pos = std::find_if(start_pos, trace.cend(),
                        [cycle](const auto& addr_cycle)
                        {
                                return addr_cycle.second == cycle;

                        });

        // Store the new start position
        const uint64_t new_pos = std::distance(trace.cbegin(), search_start_pos);
        assert(new_pos >= indices[set]);
        indices[set] = new_pos;

        // Find the first next use cycle for all the elements in the set and the fill address
        std::map<uint64_t, uint64_t> next_use_cycle;    // Key is the address, value is the first use cycle
        for(const auto& addr: set_contents)
        {
                next_use_cycle[addr] = std::find_if(search_start_pos, accesses[set].cend(),
                                [addr](const auto& addr_cycle)
                                {
                                        return addr_cycle.first == addr;
                                })->second;
        }
        next_use_cycle[fill_addr] = std::find_if(search_start_pos, accesses[set].cend(),
                        [fill_addr](const auto& addr_cycle)
                        {
                                return addr_cycle.first == fill_addr;
                        })->second;

        // Identify the victim address
        const auto& victim_address = std::max_element(next_use_cycle.cbegin(), next_use_cycle.cend(),
                        [](const auto& lhs, const auto& rhs)
                        {
                                return lhs.second < rhs.second;
                        })->first;

        // Identify the victim way
        auto way = std::find(set_contents.cbegin(), set_contents.cend(), victim_address);
        assert((way != set_contents.cend()) || victim_address == fill_addr);
        return std::distance(set_contents.cbegin(), way);
}
