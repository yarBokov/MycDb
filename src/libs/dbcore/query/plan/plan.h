#ifndef __QUERY_PLAN_H
#define __QUERY_PLAN_H

#include "libs/dbcore/scan/i_scan.h"
#include "libs/dbcore/record/schema.h"

namespace dbcore::query::plan
{
    class plan
    {
        public:
            virtual scan::i_scan open();
            virtual int blocks_accessed();
            virtual int records();
            virtual int distinct_values(const std::string& fldname) const;
            virtual record::schema schema();
    };
}

#endif