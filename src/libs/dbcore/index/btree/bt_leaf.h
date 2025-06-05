#ifndef __INDEX_BTREE_IDX_LEAF_H
#define __INDEX_BTREE_IDX_LEAF_H

#include "bt_page.h"
#include "dir_entry.h"

#include "libs/dbcore/record/layout.h"

namespace dbcore::index
{
    class btree_leaf
    {
        private:
            std::shared_ptr<tx::transaction> m_tx;
            record::layout m_layout;
            query::constant m_search_key;
            btree_page m_contents;
            int m_curr_slot;
            std::string m_filename;

            bool try_overflow()
            {
                auto first_key = m_contents.get_data_val(0);
                int flag = m_contents.get_flag();
                if (m_search_key != first_key || flag < 0)
                    return false;

                m_contents.close();
                file_mgr::block_id next_blk(m_filename, flag);
                m_contents = btree_page(m_tx, next_blk, m_layout);
                m_curr_slot = 0;
                return true;
            }

        public:
            btree_leaf(std::shared_ptr<tx::transaction> tx, 
                       const file_mgr::block_id& blk,
                       const record::layout& layout,
                       const query::constant& search_key)
                : m_tx(tx)
                , m_layout(layout)
                , m_search_key(search_key)
                , m_contents(tx, blk, layout)
                , m_filename(blk.get_filename())
            {
                m_curr_slot = m_contents.find_slot_before(search_key);
            }

            void close()
            {
                m_contents.close();
            }

            bool next()
            {
                m_curr_slot++;
                if (m_curr_slot >= m_contents.get_num_recs())
                    return try_overflow();
                else if (m_contents.get_data_val(m_curr_slot) == m_search_key)
                    return true;
                else
                    try_overflow();
            }

            record::record_id get_data_rid() const
            {
                return m_contents.get_data_rid(m_curr_slot);
            }

            void delete_record(const record::record_id& rid)
            {
                while (next())
                {
                    if (get_data_rid() == rid)
                    {
                        m_contents.delete_record(m_curr_slot);
                        return;
                    }
                }
            }

            dir_entry insert(const record::record_id& rid)
            {
                if (m_contents.get_flag() >= 0 && m_contents.get_data_val(0) > m_search_key)
                {
                    auto first_val = m_contents.get_data_val(0);
                    block_id new_blk = m_contents.split(0, m_contents.get_flag());
                    m_curr_slot = 0;
                    m_contents.set_flag(-1);
                    m_contents.insert_leaf(m_curr_slot, m_search_key, rid); 
                    return dir_entry(first_val, new_blk.get_block_number());  
                }

                m_curr_slot++;
                m_contents.insert_leaf(m_curr_slot, m_search_key);
                if (!m_contents.is_full())
                    return dir_entry{};

                // Page is full, so split it
                auto first_key = m_contents.get_data_val(0);
                auto last_key = m_contents.get_data_val(m_contents.get_num_recs() - 1);
        
                if (last_key == first_key)
                {
                    // Create overflow block
                    block_id new_blk = m_contents.split(1, m_contents.get_flag());
                    m_contents.set_flag(new_blk.get_block_number());
                    return dir_entry();
                } 
                else
                {
                    int split_pos = m_contents.get_num_recs() / 2;
                    auto split_key = m_contents.get_data_val(split_pos);
                    
                    if (split_key == first_key) {
                        // Move right looking for next key
                        while (m_contents.get_data_val(split_pos) == split_key)
                            split_pos++;
                        split_key = m_contents.get_data_val(split_pos);
                    } else {
                        // Move left looking for first entry with that key
                        while (m_contents.get_data_val(split_pos - 1) == split_key)
                            split_pos--;
                    }
                    
                    block_id new_blk = m_contents.split(split_pos, -1);
                    return dir_entry(split_key, new_blk.get_block_number());
                }
            }
    };
}

#endif