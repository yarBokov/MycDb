#ifndef __METADATA_IDX_INFO_H
#define __METADATA_IDX_INFO_H

#include "stat_info.h"
#include "libs/dbcore/record/schema.h"
#include "libs/dbcore/record/layout.h"
#include "libs/dbcore/tx/transaction.h"
#include "libs/dbcore/index/i_index.h"
#include "libs/dbcore/index/index_type.h"
#include "libs/dbcore/index/hash/hash_index.hpp"
#include "libs/dbcore/index/btree/btree_index.hpp"

namespace dbcore::metadata
{
    class index_info
    {
        private:
            std::string m_idxname;
            std::string m_fldname;
            std::shared_ptr<tx::transaction> m_tx;
            std::shared_ptr<record::schema> m_tbl_sch;
            std::unique_ptr<record::layout> m_idx_layout;
            stat_info m_stats;
            index::index_type m_idx_type;

            std::unique_ptr<record::layout> create_index_layout()
            {
                auto sch = std::make_shared<record::schema>();
                sch->add_int_field("block");
                sch->add_int_field("id");
                if (m_tbl_sch->length(m_fldname) == static_cast<int>(record::schema::sql_types::integer))
                    sch->add_int_field("dataval");
                else
                {
                    int fldlen = m_tbl_sch->length(m_fldname);
                    sch->add_str_field("dataval", fldlen);
                }
                return std::make_unique<record::layout>(sch);
            }

        public:
            index_info() = default;
            index_info(const std::string& idxname, const std::string& fldname, 
                      std::shared_ptr<record::schema> sch, std::shared_ptr<tx::transaction> tx, const stat_info& stats, index::index_type idx_type)
                : m_idxname(idxname)
                , m_fldname(fldname)
                , m_tx(tx)
                , m_tbl_sch(sch)
                , m_stats(stats)
                , m_idx_type(idx_type)
            {
                m_idx_layout = create_index_layout();
            }

            std::shared_ptr<index::i_index> open()
            {
                switch (m_idx_type)
                {
                    case index::index_type::hash_index:
                        return std::make_shared<index::hash_index>(m_tx, m_idxname, *m_idx_layout);
                    case index::index_type::binarytree_index:
                        return std:: make_shared<index::btree_index>(m_tx, m_idxname, *m_idx_layout);
                    default:
                        return nullptr;
                }
            }

            std::size_t blocks_accessed() const
            {
                int rpb = m_tx->block_size() / m_idx_layout->slot_size();
                int num_blocks = m_stats.records_output() / rpb;
                switch (m_idx_type)
                {
                    case index::index_type::hash_index:
                        return index::hash_index::search_cost(num_blocks, rpb);
                    case index::index_type::binarytree_index:
                        return index::btree_index::search_cost(num_blocks, rpb);
                    default:
                        return -1;
                }
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