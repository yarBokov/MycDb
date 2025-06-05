#include "hash_index.hpp"

namespace dbcore::index
{
    hash_index::hash_index(std::shared_ptr<tx::transaction> tx, const std::string& idxname, const record::layout& layout)
        : m_buckets_num(100), m_tx(tx), m_idxname(idxname), m_layout(layout)
    {}

    void hash_index::before_first(const query::constant& searchkey)
    {
        close();
        m_searchkey = searchkey;
        int bucket = searchkey.hash_code() % m_buckets_num;
        std::string tblname = m_idxname + std::to_string(bucket);
        m_ts = std::make_unique<record::table_scan>(m_tx, tblname, m_layout);
    }

    bool hash_index::next()
    {
        while(m_ts->next())
        {
            if (m_ts->get_val("dataval") == m_searchkey)
                return true;
        }
        return false;
    }

    record::record_id hash_index::get_data_rid()
    {
        int blknum = m_ts->get_int("block");
        int id = m_ts->get_int("id");
        return record::record_id(blknum, id);
    }

    void hash_index::insert(const query::constant& dataval, const record::record_id& datarid)
    {
        before_first(dataval);
        m_ts->insert();
        m_ts->set_int("block", datarid.block_number());
        m_ts->set_int("id", datarid.slot());
        m_ts->set_val("dataval", dataval);
    }

    void hash_index::delete_record(const query::constant& dataval, const record::record_id& datarid)
    {
        before_first(dataval);
        while(next())
        {
            if (get_data_rid() == datarid)
            {
                m_ts->delete_record();
                return;
            }
        }
    }

    void hash_index::close()
    {
        if (m_ts)
        {
            m_ts->close();
            m_ts.reset();
        }
    }

    std::size_t hash_index::search_cost(int numblocks, int rpb)
    {
        return numblocks / rpb;
    }

    void hash_index::set_buckets_num(std::size_t buckets_num)
    {
        if (buckets_num > m_buckets_num)
            m_buckets_num = buckets_num;
    }
}