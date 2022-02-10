#include <gdsb/experiment.h>

#include <sys/resource.h>

namespace gdsb
{

//! Returns the memory usage in KB for the calling process.
unsigned long long memory_usage_in_kb()
{
    rusage memory_usage;
    if (0 != getrusage(RUSAGE_SELF, &memory_usage))
    {
        throw std::runtime_error("Could not retreive memory usage.");
    }

    return memory_usage.ru_maxrss;
}

} // namespace gdsb
