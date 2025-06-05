#ifndef __QUERY_PLAN_H
#define __QUERY_PLAN_H

#include <memory>

#include "libs/dbcore/scan/i_scan.h"
#include "libs/dbcore/record/schema.h"

namespace dbcore::query::plan
{
    class i_plan
    {
        public:
            virtual std::shared_ptr<scan::i_scan> open() = 0;
            virtual std::size_t blocks_accessed() const = 0;
            virtual std::size_t records() const = 0;
            virtual std::size_t distinct_values(const std::string& fldname) const = 0;
            virtual record::schema schema() = 0;
    };
}

#endif