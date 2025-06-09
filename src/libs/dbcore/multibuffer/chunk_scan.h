#ifndef __MULTIBUFFER_CHUNK_SCAN_H
#define __MULTIBUFFER_CHUNK_SCAN_H

#include "libs/dbcore/scan/i_scan.h"
#include "libs/dbcore/tx/transaction.h"
#include "libs/dbcore/record/record_page.h"
#include "libs/dbcore/record/layout.h"

#include <vector>
#include <memory>
#include <string>

using namespace dbcore::scan;

namespace dbcore::multibuffer
{
    class chunk_scan : public i_scan
    {
        private:
            std::shared_ptr<tx::transaction> m_tx;
            std::string m_filename;
            record::layout m_layout;
            int m_start_blk_num;
            int m_end_blk_num;
            int m_curr_blk_num;
            std::vector<std::unique_ptr<record::record_page>> m_pages;
            record::record_page* m_rp;
            int m_curr_slot;

            void move_to_blk(int blk_num)
            {
                m_curr_blk_num = blk_num;
                m_rp = m_pages[m_curr_blk_num - m_start_blk_num].get();
                m_curr_slot = -1;
            }

        public:
            chunk_scan(
                std::shared_ptr<tx::transaction> tx, 
                const std::string& filename,
                const record::layout& layout,
                int start_blk_num,
                int end_blk_num)
                : m_tx(tx)
                , m_filename(filename)
                , m_layout(layout)
                , m_start_blk_num(start_blk_num)
                , m_end_blk_num(end_blk_num)
            {
                for (int i = m_start_blk_num; i <= m_end_blk_num; i++)
                {
                    file_mgr::block_id blk(m_filename, i);
                    m_pages.push_back(std::make_unique<record::record_page>(m_tx, blk, m_layout));
                }
                move_to_blk(m_start_blk_num);;
            }

            void before_first() override
            {
                move_to_blk(m_start_blk_num);
            }

            bool next() override
            {
                m_curr_slot = m_rp->next_after(m_curr_slot);
                while (m_curr_slot < 0)
                {
                    if (m_curr_blk_num == m_end_blk_num)
                        return false;

                    move_to_blk(m_rp->block().get_block_number() + 1);
                    m_curr_slot = m_rp->next_after(m_curr_slot);
                }
                return true;
            }

            int get_int(const std::string& fldname) override
            {
                return m_rp->get_int(m_curr_slot, fldname);
            }

            std::string get_str(const std::string& fldname) override
            {
                return m_rp->get_str(m_curr_slot, fldname);
            }

            query::constant get_val(const std::string& fldname) override
            {
                if (m_layout.get_schema()->type(fldname) == dbcore::record::schema::sql_types::integer)
                    return query::constant(get_int(fldname));
                else
                    return query::constant(get_str(fldname));
            }

            bool has_field(const std::string& fldname) override
            {
                return m_layout.get_schema()->has_field(fldname);
            }

            void close() override
            {
                for (auto i = 0; i < m_pages.size(); i++)
                {
                    file_mgr::block_id blk(m_filename, m_start_blk_num + static_cast<int>(i));
                    m_tx->unpin(blk);
                }
            }
    };
}

#endif