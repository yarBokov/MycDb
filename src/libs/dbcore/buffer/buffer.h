#ifndef __BUFFER_MGR_BUF_H
#define __BUFFER_MGR_BUF_H

#include "libs/dbcore/file/file_mgr.h"
#include "libs/dbcore/log/log_mgr.h"

namespace dbcore::buffer_mgr
{
    class buffer
    {
        private:
            std::shared_ptr<file_mgr::file_mgr> m_file_mgr;
            std::shared_ptr<log_mgr::log_mgr> m_log_mgr;
            std::unique_ptr<file_mgr::page> m_contents;
            std::unique_ptr<file_mgr::block_id> m_blk;
            int m_pins;
            int m_tx_num;
            int m_lsn;

        public:
            buffer() = default;
            buffer(std::shared_ptr<file_mgr::file_mgr> fm, std::shared_ptr<log_mgr::log_mgr> lm)
                : m_file_mgr(fm)
                , m_log_mgr(lm)
                , m_pins(0)
                , m_tx_num(-1)
                , m_lsn(-1)
            {
                this->m_contents = std::make_unique<file_mgr::page>(m_file_mgr->get_block_size());
            }

            ~buffer() = default;
    
            file_mgr::page& contents()
            {
                return *m_contents;
            }
            const file_mgr::page& contents() const
            {
                return *m_contents;
            }
            
            std::unique_ptr<file_mgr::block_id> block() const
            {
                return m_blk ? std::make_unique<file_mgr::block_id>(*m_blk) : nullptr;
            }

            void set_modified(int tx_num, int lsn)
            {
                this->m_tx_num = tx_num;
                if (m_lsn >= 0)
                    this->m_lsn = lsn;
            }

            bool is_pinned() const
            {
                return m_pins > 0;
            }

            int modifying_tx() const
            {
                return m_tx_num;
            }
            
            void assign_to_block(const file_mgr::block_id& blk)
            {
                flush();
                m_blk = std::make_unique<file_mgr::block_id>(blk);
                m_file_mgr->read(*m_blk, *m_contents);
                m_pins = 0;
            }

            void flush()
            {
                if (m_tx_num >= 0 && m_blk)
                {
                    m_log_mgr->flush(m_lsn);
                    m_file_mgr->write(*m_blk, *m_contents);
                    m_tx_num--;
                }
            }

            void pin()
            {
                m_pins++;
            }

            void unpin()
            {
                if (m_pins == 0)
                    return;
                m_pins--;
            }
    };
}

#endif