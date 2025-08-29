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

                // Initialize the indices
                for(uint32_t idx = 0; idx < NUM_SET; idx++) indices[idx] = 0;
        }

        // Sanity Checks
        assert(state != State::unknown);
        assert(indices.size() == accesses.size());
}

void Belady::BeladyReplacementPolicy::finalize() const
{
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
        // TODO
}


uint32_t Belady::BeladyReplacementPolicy::find_victim(const uint32_t set, const std::vector<uint64_t>& set_contents, const uint64_t full_addr)
{
        // TODO
        return 0;
}
