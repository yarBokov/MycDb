#ifndef __MATERIALIZE_AGGREGATE_IFACE_FN_H
#define __MATERIALIZE_AGGREGATE_IFACE_FN_H

#include "libs/dbcore/query/constant.h"
#include "libs/dbcore/scan/i_scan.h"
#include "field_names_prefixes.h"

namespace dbcore::materialize
{
    class i_aggregate_fn
    {
        public:
            virtual ~i_aggregate_fn() = default;

            virtual void process_first(std::shared_ptr<scan::i_scan> s) = 0;
            virtual void process_next(std::shared_ptr<scan::i_scan> s) = 0;
            virtual std::string field_name() const = 0;
            virtual query::constant value() const = 0;
    };
}

#endif