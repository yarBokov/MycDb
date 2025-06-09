#ifndef __RECORD_RECORDPAGE_H
#define __RECORD_RECORDPAGE_H

#include "layout.h"
#include "libs/dbcore/tx/transaction.h"

namespace dbcore::record
{
    class record_page
    {
        public:
            enum class page_rec_flag
            {
                empty = 0,
                used
            };

        private:
            std::shared_ptr<tx::transaction> m_tx;
            file_mgr::block_id& m_blk;
            layout& m_layout;

            int calculate_offset(int slot) const
            {
                return slot * m_layout.slot_size();
            }

            void set_flag(int slot, page_rec_flag flag)
            {
                m_tx->set_int(m_blk, calculate_offset(slot), static_cast<int>(flag), true);
            }

            bool is_valid_slot(int slot) const
            {
                return calculate_offset(slot + 1) <= m_tx->block_size();
            }

            int search_after(int slot, page_rec_flag flag)
            {
                slot++;
                while (is_valid_slot(slot))
                {
                    if (m_tx->get_int(m_blk, calculate_offset(slot)) == static_cast<int>(flag))
                        return slot;
                    slot++;
                }
                return -1;
            }

        public:
            record_page(std::shared_ptr<tx::transaction> tx, file_mgr::block_id& blk, layout& layout)
                : m_tx(tx), m_blk(blk), m_layout(layout)
            {
                m_tx->pin(m_blk);
            }
    
            int get_int(int slot, const std::string& fldname)
            {
                int fldpos = calculate_offset(slot) + m_layout.offset(fldname);
                return m_tx->get_int(m_blk, fldpos);
            }

            std::string get_str(int slot, const std::string& fldname)
            {
                int fldpos = calculate_offset(slot) + m_layout.offset(fldname);
                return m_tx->get_str(m_blk, fldpos);
            }
            
            void set_int(int slot, const std::string& fldname, int value)
            {
                int fldpos = calculate_offset(slot) + m_layout.offset(fldname);
                m_tx->set_int(m_blk, fldpos, value, true);
            }

            void set_str(int slot, const std::string& fldname, const std::string& value)
            {
                int fldpos = calculate_offset(slot) + m_layout.offset(fldname);
                m_tx->set_str(m_blk, fldpos, value, true);
            }
            
            void delete_record(int slot)
            {
                set_flag(slot, page_rec_flag::empty);
            }

            void format()
            {
                int slot = 0;
                while (is_valid_slot(slot))
                {
                    m_tx->set_int(m_blk, calculate_offset(slot), static_cast<int>(page_rec_flag::empty), false);
                    auto sch = m_layout.get_schema();
                    for (const auto& fldname : sch->fields())
                    {
                        int fldpos = calculate_offset(slot) + m_layout.offset(fldname);
                        if (sch->type(fldname) == schema::sql_types::integer)
                            m_tx->set_int(m_blk, fldpos, 0, false);
                        else
                            m_tx->set_str(m_blk, fldpos, "", false);
                    }
                    slot++;
                }
            }
            
            int next_after(int slot)
            {
                return search_after(slot, page_rec_flag::used);
            }

            int insert_after(int slot)
            {
                int new_slot = search_after(slot, page_rec_flag::empty);
                if (new_slot >= 0)
                    set_flag(new_slot, page_rec_flag::used);
                return new_slot;
            }
            
            file_mgr::block_id& block() const
            {
                return m_blk;
            }
    };
}

#endif