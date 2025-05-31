#ifndef __QUERY_I_QUERY_PLANNER_H
#define __QUERY_i_QUERY_PLANNER_H

#include "libs/dbcore/query/plan/i_plan.h"
#include "libs/dbcore/parse/ops_data/query_data.h"
#include "libs/dbcore/tx/transaction.h"

#include <memory>

namespace dbcore::query
{
    class i_query_planner
    {
        public:
            virtual std::unique_ptr<plan::i_plan> create_plan(const parse::query_data& data, tx::transaction& tx);
    };
}

#endif