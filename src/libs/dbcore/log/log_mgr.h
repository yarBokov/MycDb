#ifndef __LOG_MGR_IMPL_H
#define __LOG_MGR_IMPL_H

#include "log_iterator.h"

namespace dbcore::log_mgr
{
    class log_mgr
    {
        private:
            using f_m = dbcore::file_mgr::file_mgr;
            using blk_id = dbcore::file_mgr::block_id;
            using page = dbcore::file_mgr::page;

            std::shared_ptr<f_m> m_fm;
            std::string m_log_file;
            std::unique_ptr<page> m_log_page;
            blk_id m_cur_blk;
            int m_latest_LSN;
            int m_last_saved_LSN;
            mutable std::mutex m_log_guard;

            blk_id append_new_block()
            {
                auto blk = m_fm->append(m_log_file);

                int aligned_boundary = (m_fm->get_block_size() / sizeof(int)) * sizeof(int);
                m_log_page->set_int(0, aligned_boundary);
                m_fm->write(*blk, *m_log_page);
                return *blk;
            }

            void flush_internal()
            {
                m_fm->write(m_cur_blk, *m_log_page);
                m_last_saved_LSN = m_latest_LSN;
            }

        public:
            log_mgr() = default;
            ~log_mgr() = default;

            log_mgr(std::shared_ptr<f_m> fm, const std::string& log_file)
                : m_fm(fm)
                , m_log_file(log_file)
                , m_last_saved_LSN{0}
                , m_latest_LSN{0}
            {
                std::vector<char> buf(m_fm->get_block_size(), 0);
                m_log_page = std::make_unique<page>(buf);

                int log_size = m_fm->len(m_log_file);
                if (log_size)
                    m_cur_blk = append_new_block();
                else
                {
                    m_cur_blk = blk_id(m_log_file, log_size - 1);
                    m_fm->read(m_cur_blk, *m_log_page);
                }
            }

            void flush(int lsn)
            {
                //std::lock_guard<std::mutex> lock(m_log_guard);
                if (lsn >= m_latest_LSN)
                    flush_internal();
            }

            log_iterator iterator()
            {
                //std::lock_guard<std::mutex> lock(m_log_guard);
                flush_internal();
                return log_iterator(m_fm, m_cur_blk);
            }

            int append(const std::vector<char>& log_rec)
            {
                std::lock_guard<std::mutex> lock(m_log_guard);

                const int int_size = static_cast<int>(sizeof(int));

                int boundary = m_log_page->get_int(0);
                int rec_size = static_cast<int>(log_rec.size());
                int bytes_needed = rec_size + int_size;

                if (boundary - bytes_needed < static_cast<int>(sizeof(int)))
                {
                    flush_internal();
                    m_cur_blk = append_new_block();
                    boundary = m_log_page->get_int(0);
                }

                int rec_pos = boundary - bytes_needed;

                if (rec_pos % int_size != 0)
                    throw std::runtime_error("Failed to align log record position, src: log_mgr.h");

                m_log_page->set_bytes(rec_pos, log_rec);
                m_log_page->set_int(0, rec_pos);
                
                m_latest_LSN++;
                return m_latest_LSN;
            }
    };
}

#endif