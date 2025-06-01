#ifndef __INDEX_IDX_HASH_HPP
#define __INDEX_IDX_HASH_HPP

#include "libs/dbcore/index/i_index.h"
#include "libs/dbcore/tx/transaction.h"
#include "libs/dbcore/record/layout.h"
#include "libs/dbcore/record/table_scan.h"
#include "libs/dbcore/query/constant.h"

namespace dbcore::index
{
    class hash_index : public i_index
    {
        private:
            std::size_t m_buckets_num;
            std::shared_ptr<tx::transaction> m_tx;
            std::string m_idxname;
            record::layout m_layout;
            query::constant m_searchkey;
            std::unique_ptr<record::table_scan> m_ts;

        public:
            hash_index(std::shared_ptr<tx::transaction> tx, const std::string& idxname, const record::layout& layout);
            ~hash_index() = default;

            void before_first(const query::constant& searchkey) override;
            bool next() override;
            record::record_id get_data_rid() override;
            void insert(const query::constant& dataval, const record::record_id& datarid) override;
            void delete_record(const query::constant& dataval, const record::record_id& datarid) override;
            void close() override;

            static int search_cost(int numblocks, int rpb);
            void set_buckets_num(std::size_t buckets_num);
    };
}

#endif