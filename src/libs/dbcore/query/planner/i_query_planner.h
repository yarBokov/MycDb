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
            virtual std::shared_ptr<plan::i_plan> create_plan(parse::query_data& data, std::shared_ptr<tx::transaction> tx);
    };
}

#endif