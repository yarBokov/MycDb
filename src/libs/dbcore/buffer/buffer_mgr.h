#ifndef __BUFFER_MGR_IMPL_H
#define __BUFFER_MGR_IMPL_H

#include "exceptions/buffer_abort_exception.h"
#include "buffer.h"
#include "libs/dbcore/file/file_mgr.h"
#include "libs/dbcore/log/log_mgr.h"

#include <chrono>
#include <condition_variable>

namespace dbcore::buffer_mgr
{
    class buffer_mgr
    {
        private:
            std::vector<std::unique_ptr<buffer>> buffer_pool; 
            int num_available;
            static constexpr auto MAX_TIMEOUT = std::chrono::milliseconds(10000);

            std::mutex buffer_mtx;
            std::condition_variable buffer_available;

            bool timeout_expired(const std::chrono::steady_clock::time_point& start_time) const;
            buffer* try_pin(const block_id& blk);
            buffer* find_existing_buffer(const block_id& blk);
            buffer* choose_unpinned_buffer();
            
        public:
            buffer_mgr() = default;
            explicit buffer_mgr(std::shared_ptr<file_mgr::file_mgr> fm, 
                std::shared_ptr<log_mgr::log_mgr> lm, int num_bufs);

            int get_available_count();
            void flush_all(int tx_num);
            void unpin(buffer& buf);
            buffer& pin(const block_id& blk);
    };
}

#endif