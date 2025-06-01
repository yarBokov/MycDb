#ifndef __QUERY_BASIC_UPDATE_PLANNER_H
#define __QUERY_BASIC_UPDATE_PLANNER_H

#include "i_update_planner.h"
#include "libs/dbcore/metadata/metadata_mgr.h"
#include "libs/dbcore/query/plan/table_plan.h"
#include "libs/dbcore/query/plan/select_plan.h"

namespace dbcore::query
{
    class basic_update_planner : public i_update_planner
    {
        private:
            metadata::metadata_mgr& m_mdm;

        public:
            basic_update_planner(metadata::metadata_mgr& mdm)
                : m_mdm(mdm)
            {}

            std::size_t execute_insert(parse::insert_data& data, tx::transaction& tx) override
            {
                auto p = std::make_unique<plan::table_plan>(tx, data.table_name(), m_mdm);
                auto us = dynamic_cast<scan::i_update_scan&>(*p->open());
                us.insert();
                int i = 0;
                auto consts = data.vals();
                for (const auto& fldname : data.fields())
                {
                    us.set_val(fldname, consts[i++]);
                }
                us.close();
                return 1;
            }

            std::size_t execute_delete(parse::delete_data& data, tx::transaction& tx) override
            {
                auto p = std::make_unique<plan::select_plan>(
                    std::move(std::make_unique<plan::table_plan>(tx, data.table_name(), m_mdm)), data.pred());
                auto us = dynamic_cast<scan::i_update_scan&>(*p->open());
                int count = 0;
                while(us.next())
                {
                    us.delete_record();
                    count++;
                }
                us.close();
                return count;
            }

            std::size_t execute_modify(parse::modify_data& data, tx::transaction& tx) override
            {
                auto p = std::make_unique<plan::select_plan>(
                    std::move(std::make_unique<plan::table_plan>(tx, data.table_name(), m_mdm)), data.pred());
                auto us = dynamic_cast<scan::i_update_scan&>(*p->open());
                int count = 0;
                while (us.next())
                {
                    constant val = data.new_value().evaluate(us);
                    us.set_val(data.target_field(), val);
                    count++;
                }
                us.close();
                return count;
            }

            std::size_t execute_create_table(parse::create_table_data& data, tx::transaction& tx) override
            {
                auto sch = data.new_schema();
                m_mdm.create_table(data.table_name(), sch, tx);
                return 0;
            }

            std::size_t execute_create_view(parse::create_view_data& data, tx::transaction& tx) override
            {
                m_mdm.create_view(data.view_name(), data.view_def(), tx);
                return 0;
            }

            std::size_t execute_create_index(parse::create_index_data& data, tx::transaction& tx) override
            {
                m_mdm.create_index(data.index_name(), data.table_name(), data.field_name(), tx);
                return 0;
            }
    };
}

#endif