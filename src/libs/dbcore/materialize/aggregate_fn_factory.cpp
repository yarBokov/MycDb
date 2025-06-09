#include "aggregate_fn_factory.h"

namespace dbcore::materialize
{
    std::unique_ptr<i_aggregate_fn> aggregate_fn_factory::create_max_func(const std::string& fldname)
    {
        return std::make_unique<cmp_fn>(
            fldname,
            field_prefixes::max_fn_prefix,
            [](const query::constant& c1, const query::constant& c2) { return c1 > c2; },
            query::constant(std::numeric_limits<int>::min())
        );
    }

    std::unique_ptr<i_aggregate_fn> aggregate_fn_factory::create_min_func(const std::string& fldname)
    {
        return std::make_unique<cmp_fn>(
            fldname,
            field_prefixes::min_fn_prefix,
            [](const query::constant& c1, const query::constant& c2) { return c1 < c2; },
            query::constant(std::numeric_limits<int>::max())
        );
    }

    std::unique_ptr<i_aggregate_fn> aggregate_fn_factory::create_count_func(const std::string& fldname)
    {
        return std::make_unique<count_fn>(fldname);
    }

    std::unique_ptr<i_aggregate_fn> aggregate_fn_factory::create_custom_compare_func(const std::string& fldname,
        const std::string& prefix,
        const std::function<bool(const query::constant&, const query::constant&)>& cmp_func,
        const query::constant& initial_val)
    {
        return std::make_unique<cmp_fn>(fldname, prefix, cmp_func, initial_val);
    }
}