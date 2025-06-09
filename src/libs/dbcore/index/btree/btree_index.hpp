#ifndef __INDEX_BTREE_IDX_HPP
#define __INDEX_BTREE_IDX_HPP

#include "libs/dbcore/index/i_index.h"

#include "bt_leaf.h"
#include "bt_dir.h"

namespace dbcore::index
{
    class btree_index : public i_index
    {
        private:
            std::shared_ptr<tx::transaction> m_tx;
            record::layout m_dir_layout;
            record::layout m_leaf_layout;
            std::string m_leaf_tbl;
            std::unique_ptr<btree_leaf> m_leaf;
            file_mgr::block_id m_root_blk;

        public:
            btree_index(std::shared_ptr<tx::transaction> tx, const std::string& idxname, const record::layout& leaf_layout);
            ~btree_index() = default;

            void before_first(const query::constant& searchkey) override;
            bool next() override;
            record::record_id get_data_rid() override;
            void insert(const query::constant& dataval, const record::record_id& datarid) override;
            void delete_record(const query::constant& dataval, const record::record_id& datarid) override;
            void close() override;

            static std::size_t search_cost(int numblocks, int rpb);
    };
}

#endif