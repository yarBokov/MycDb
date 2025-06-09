#ifndef __QUERY_TABLE_PLAN_H
#define __QUERY_TABLE_PLAN_H

#include "i_plan.h"

#include "libs/dbcore/tx/transaction.h"
#include "libs/dbcore/record/layout.h"
#include "libs/dbcore/record/schema.h"
#include "libs/dbcore/metadata/stat_info.h"
#include "libs/dbcore/metadata/metadata_mgr.h"

namespace dbcore::query::plan
{
    class table_plan : public i_plan
    {
        private:
            std::string m_tblname;
            std::shared_ptr<tx::transaction> m_tx;
            record::layout m_layout;
            metadata::stat_info m_stat;

        public:
            explicit table_plan(std::shared_ptr<tx::transaction> tx, const std::string& tblname, metadata::metadata_mgr& md)
                : m_tx(tx)
                , m_tblname(tblname)
                , m_layout(*md.get_layout(m_tblname, tx))
                , m_stat(*md.get_stat_info(m_tblname, m_layout, tx))
            {}

            std::shared_ptr<scan::i_scan> open() override
            {
                return std::make_shared<record::table_scan>(m_tx, m_tblname, m_layout);
            }

            std::size_t blocks_accessed() const override
            {
                return m_stat.blocks_accessed();
            }

            std::size_t records() const override
            {
                return m_stat.records_output();
            }

            std::size_t distinct_values(const std::string& fldname) const override
            {
                return m_stat.distinct_values(fldname);
            }

            std::shared_ptr<record::schema> schema() override
            {
                return m_layout.get_schema();
            }
    };
}

#endif