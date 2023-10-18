#include <gdsb/graph.h>

#include <limits>

namespace gdsb
{

Weight invalid_weight() { return std::numeric_limits<Weight>::infinity(); }

} // namespace gdsb
