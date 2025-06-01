#ifndef __QUERY_I_UPDATE_PLANNER_H
#define __QUERY_I_UPDATE_PLANNER_H

#include "libs/dbcore/parse/ops_data/insert_data.h"
#include "libs/dbcore/parse/ops_data/delete_data.h"
#include "libs/dbcore/parse/ops_data/modify_data.h"
#include "libs/dbcore/parse/ops_data/create_table_data.h"
#include "libs/dbcore/parse/ops_data/create_view_data.h"
#include "libs/dbcore/parse/ops_data/create_index_data.h"
#include "libs/dbcore/tx/transaction.h"

namespace dbcore::query
{
    class i_update_planner
    {
        public:
            virtual std::size_t execute_insert(parse::insert_data& data, tx::transaction& tx);
            virtual std::size_t execute_delete(parse::delete_data& data, tx::transaction& tx);
            virtual std::size_t execute_modify(parse::modify_data& data, tx::transaction& tx);
            virtual std::size_t execute_create_table(parse::create_table_data& data, tx::transaction& tx);
            virtual std::size_t execute_create_view(parse::create_view_data& data, tx::transaction& tx);
            virtual std::size_t execute_create_index(parse::create_index_data& data, tx::transaction& tx);
    };

}

#endif