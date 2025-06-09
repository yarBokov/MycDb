#include "btree_index.hpp"

namespace dbcore::index
{
    btree_index::btree_index(std::shared_ptr<tx::transaction> tx, const std::string& idxname, const record::layout& leaf_layout)
        : m_tx(tx), m_leaf_layout(leaf_layout)
    {
        m_leaf_tbl = idxname + "leaf";
        if (m_tx->size(m_leaf_tbl) == 0)
        {
            auto blk = m_tx->append(m_leaf_tbl);
            btree_page node(m_tx, *blk, m_leaf_layout);
            node.format(*blk, -1);
        }

    }

    void btree_index::before_first(const query::constant& searchkey)
    {
        close();
        btree_dir root(m_tx, m_root_blk, m_dir_layout);
        int blk_num = root.search(searchkey);
        root.close();
        file_mgr::block_id leaf_blk(m_leaf_tbl, blk_num);
        m_leaf = std::make_unique<btree_leaf>(m_tx, leaf_blk, m_leaf_layout, searchkey);
    }

    bool btree_index::next()
    {
        return m_leaf ? m_leaf->next() : false;
    }

    record::record_id btree_index::get_data_rid()
    {
        return m_leaf ? m_leaf->get_data_rid() : record::record_id();
    }

    void btree_index::insert(const query::constant& dataval, const record::record_id& datarid)
    {
        before_first(dataval);
        dir_entry e = m_leaf->insert(datarid);
        m_leaf->close();
        m_leaf.reset();

        if (e.block_number() == -1)
            return;

        btree_dir root(m_tx, m_root_blk, m_dir_layout);
        dir_entry e2 = root.insert(e);
        if (e2.block_number() == -1)
            root.make_new_root(e2);

        root.close();
    }

    void btree_index::delete_record(const query::constant& dataval, const record::record_id& datarid)
    {
        before_first(dataval);
        m_leaf->delete_record(datarid);
        m_leaf->close();
        m_leaf.reset();
    }

    void btree_index::close()
    {
        if (m_leaf)
        {
            m_leaf->close();
            m_leaf.reset();
        }
    }

    std::size_t btree_index::search_cost(int numblocks, int rpb)
    {
        return 1 + static_cast<std::size_t>(std::log(numblocks) / std::log(rpb));
    }
}