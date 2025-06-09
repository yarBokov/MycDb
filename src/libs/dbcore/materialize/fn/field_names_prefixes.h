#ifndef __MATERIALIZE_AGGREGATE_FN_PREFIXES_H
#define __MATERIALIZE_AGGREGATE_FN_PREFIXES_H

#include <string>

namespace dbcore::materialize::field_prefixes
{
    const std::string count_fn_prefix = "CountOf_";
    const std::string max_fn_prefix = "MaxOf_";
    const std::string min_fn_prefix = "MinOf_";
}

#endif