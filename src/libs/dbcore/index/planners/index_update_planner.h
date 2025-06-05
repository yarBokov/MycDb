#ifndef __INDEX_IDX_UPDATE_PLANNER_H
#define __INDEX_IDX_UPDATE_PLANNER_H

#include "libs/dbcore/query/planner/i_update_planner.h"
#include "libs/dbcore/metadata/metadata_mgr.h"
#include "libs/dbcore/query/plan/table_plan.h"
#include "libs/dbcore/query/plan/select_plan.h"

using namespace dbcore::parse;

namespace dbcore::index
{
    class index_update_planner : public query::i_update_planner
    {
        private:
            metadata::metadata_mgr& m_mdm;

        public:
            explicit index_update_planner(metadata::metadata_mgr& mdm)
                : m_mdm(mdm)
            {}

            std::size_t execute_insert(parse::insert_data& data, tx::transaction& tx) override
            {
                const std::string& tblname = data.table_name();
                auto p = std::make_unique<query::plan::table_plan>(tx, tblname, m_mdm);

                auto s = dynamic_cast<scan::i_update_scan*>(p->open().get());
                if (!s)
                    throw std::runtime_error("Expected update_scan, 'index_update_planner: insertion'");

                s->insert();
                auto rid = s->get_rid();

                auto indexes = m_mdm.get_index_info(tblname, tx);
                auto val_tier = data.vals().begin();
                for (const auto& fldname : data.fields())
                {
                    query::constant val = *val_tier++;
                    s->set_val(fldname, val);

                    if (indexes.count(fldname))
                    {
                        auto idx = indexes.at(fldname).open();
                        idx->insert(val, rid);
                        idx->close();
                    }
                }
                s->close();
                return 1;
            }

            std::size_t execute_delete(parse::delete_data& data, tx::transaction& tx) override
            {
                const std::string& tblname = data.table_name();
                auto p = std::make_unique<query::plan::select_plan>(
                    std::make_unique<query::plan::table_plan>(tx, tblname, m_mdm), data.pred());
                auto indexes = m_mdm.get_index_info(tblname, tx);

                auto s = dynamic_cast<scan::i_update_scan*>(p->open().get());
                if (!s)
                    throw std::runtime_error("Expected update_scan, 'index_update_planner: deletion'");

                int count = 0;
                while (s->next())
                {
                    auto rid = s->get_rid();
                    for (auto& [fldname, ii] : indexes)
                    {
                        query::constant val = s->get_val(fldname);
                        auto idx = ii.open();
                        idx->delete_record(val, rid);
                        idx->close();
                    }

                    s->delete_record();
                    count++;
                }
                s->close();
                return count;
            }

            std::size_t execute_modify(parse::modify_data& data, tx::transaction& tx) override
            {
                const std::string& tblname = data.table_name();
                const std::string& fldname = data.target_field();
                auto p = std::make_unique<query::plan::select_plan>(
                    std::make_unique<query::plan::table_plan>(tx, tblname, m_mdm), data.pred());

                auto indexes = m_mdm.get_index_info(tblname, tx);
                std::shared_ptr<i_index> idx;
                if (indexes.count(fldname))
                    idx = indexes.at(fldname).open();

                auto s = dynamic_cast<scan::i_update_scan*>(p->open().get());
                if (!s)
                    throw std::runtime_error("Expected update_scan, 'index_update_planner: modification'");

                int count = 0;
                while (s->next())
                {
                    query::constant newval = data.new_value().evaluate(*s);
                    query::constant oldval = s->get_val(fldname);

                    s->set_val(fldname, newval);

                    if (idx)
                    {
                        auto rid = s->get_rid();
                        idx->delete_record(oldval, rid);
                        idx->insert(newval, rid);
                    }
                    count++;
                }
                if (idx)
                    idx->close();

                s->close();
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