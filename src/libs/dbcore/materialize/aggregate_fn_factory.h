#ifndef __MATERIALIZE_AGGREGATE_FN_FACTORY_H
#define __MATERIALIZE_AGGREGATE_FN_FACTORY_H

#include "fn/cmp_fn.h"
#include "fn/count_fn.h"
#include "fn/i_aggregate_fn.h"
#include <memory>

namespace dbcore::materialize
{
    class aggregate_fn_factory
    {
        public:
            static std::unique_ptr<i_aggregate_fn> create_max_func(const std::string& fldname);
            static std::unique_ptr<i_aggregate_fn> create_min_func(const std::string& fldname);
            static std::unique_ptr<i_aggregate_fn> create_count_func(const std::string& fldname);
            static std::unique_ptr<i_aggregate_fn> create_custom_compare_func(
                const std::string& fldname,
                const std::string& prefix,
                const std::function<bool(const query::constant&, const query::constant&)>& cmp_func,
                const query::constant& initial_val);
    };
}

#endif