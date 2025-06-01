#ifndef __METADATA_IDX_INFO_H
#define __METADATA_IDX_INFO_H

#include "stat_info.h"
#include "libs/dbcore/record/schema.h"
#include "libs/dbcore/record/layout.h"
#include "libs/dbcore/tx/transaction.h"
#include "libs/dbcore/index/index_type.h"

namespace dbcore::metadata
{
    class index_info
    {
        private:
            std::string m_idxname;
            std::string m_fldname;
            tx::transaction& m_tx;
            std::unique_ptr<record::schema> m_tbl_sch;
            std::unique_ptr<record::layout> m_idx_layout;
            stat_info m_stats;

            std::unique_ptr<record::layout> create_index_layout()
            {
                auto sch = std::make_unique<record::schema>();
                sch->add_int_field("block");
                sch->add_int_field("id");
                if (m_tbl_sch->length(m_fldname) == static_cast<int>(record::schema::sql_types::integer))
                    sch->add_int_field("dataval");
                else
                {
                    int fldlen = m_tbl_sch->length(m_fldname);
                    sch->add_str_field("dataval", fldlen);
                }
                return std::make_unique<record::layout>(*sch);
            }

        public:
            index_info() = default;
            index_info(const std::string& idxname, const std::string& fldname, 
                      std::unique_ptr<record::schema> sch, tx::transaction& tx, const stat_info& stats)
                : m_idxname(idxname)
                , m_fldname(fldname)
                , m_tx(tx)
                , m_tbl_sch(std::move(sch))
                , m_stats(stats)
            {
                m_idx_layout = create_index_layout();
            }

            void open() //TODO: Change return type when index class will be ready
            {
                return ;
                // return new btree_index();
            }

            std::size_t blocks_accessed() const
            {
                int rpb = m_tx.block_size() / m_idx_layout->slot_size();
                int num_blocks = m_stats.records_output() / rpb;
                return 0;
                //TODO: change to index->search_cost();
            }

            std::size_t records_output() const
            {
                return m_stats.records_output() / m_stats.distinct_values(m_fldname);
            }

            std::size_t distinct_values(const std::string& fldname)
            {
                return (fldname == m_fldname ? 1 : m_stats.distinct_values(fldname));
            }
    };
}

#endif