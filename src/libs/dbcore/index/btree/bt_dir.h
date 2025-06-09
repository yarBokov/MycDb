#ifndef __INDEX_BTREE_DIR_H
#define __INDEX_BTREE_DIR_H

#include "dir_entry.h"
#include "bt_page.h"

namespace dbcore::index
{
    class btree_dir
    {
        private:
            std::shared_ptr<tx::transaction> m_tx;
            record::layout m_layout;
            btree_page m_contents;
            std::string m_filename;

            dir_entry insert_entry(const dir_entry& e)
            {
                int new_slot = 1 + m_contents.find_slot_before(e.data_val());
                m_contents.insert_dir(new_slot, e.data_val(), e.block_number());

                if (!m_contents.is_full())
                    return dir_entry();

                int level = m_contents.get_flag();
                int split_pos = m_contents.get_num_recs() / 2;
                query::constant split_val = m_contents.get_data_val(split_pos);
                file_mgr::block_id new_blk = m_contents.split(split_pos, level);
                return dir_entry(split_val, new_blk.get_block_number());
            }

            file_mgr::block_id find_child_block(const query::constant& search_key)
            {
                auto slot = m_contents.find_slot_before(search_key);
                if (m_contents.get_data_val(slot + 1) == search_key)
                    slot++;
                int blk_num = m_contents.get_child_num(slot);
                return file_mgr::block_id(m_filename, blk_num);
            }

        public:
            btree_dir(std::shared_ptr<tx::transaction> tx, 
                const file_mgr::block_id& blk, 
                const record::layout& layout)
                : m_tx(tx)
                , m_layout(layout)
                , m_contents(m_tx, blk, layout)
                , m_filename(blk.get_filename())
            {}
        
            void close()
            {
                m_contents.close();
            }

            int search(const query::constant& search_key)
            {
                auto child_blk = find_child_block(search_key);
                while (m_contents.get_flag() > 0)
                {
                    m_contents.close();
                    m_contents = btree_page(m_tx, child_blk, m_layout);
                    child_blk = find_child_block(search_key);
                }
                return child_blk.get_block_number();
            }

            void make_new_root(const dir_entry& e)
            {
                auto first_val = m_contents.get_data_val(0);
                int level = m_contents.get_flag();
                auto new_blk = m_contents.split(0, level);
                dir_entry old_root(first_val, new_blk.get_block_number());
                insert_entry(old_root);
                insert_entry(e);
                m_contents.set_flag(level + 1);
            }
            
            dir_entry insert(const dir_entry& e)
            {
                if (m_contents.get_flag() == 0)
                {
                    return insert_entry(e);
                }

                file_mgr::block_id child_blk = find_child_block(e.data_val());
                btree_dir child(m_tx, child_blk, m_layout);
                dir_entry my_entry = child.insert(e);
                child.close();
                return my_entry.block_number() != 1 ? insert_entry(my_entry) : dir_entry();
            }
    };
}

#endif