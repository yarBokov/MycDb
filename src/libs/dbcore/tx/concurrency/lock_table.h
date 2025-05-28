#ifndef __TX_CONCURRENCY_LOCK_TABLE_H
#define __TX_CONCURRENCY_LOCK_TABLE_H

#include "libs/dbcore/file/page/block_id.h"

#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include "libs/dbcore/exceptions/lock_abort_exception.h"

namespace dbcore::tx
{
    class lock_table
    {
        private:
            std::unordered_map<file_mgr::block_id, int, file_mgr::block_id::hash> locks;
            mutable std::mutex mtx;
            std::condition_variable cond;
            
            static constexpr auto MAX_TIMEOUT = std::chrono::milliseconds(10000);

            int get_lock_value(const file_mgr::block_id& blk) const
            {
                auto it = locks.find(blk);
                return (it == locks.end()) ? 0 : it->second;
            }

            bool has_x_lock(const file_mgr::block_id& blk) const
            {
                return get_lock_value(blk) < 0;
            }

            bool has_s_locks(const file_mgr::block_id& blk) const
            {
                return get_lock_value(blk) > 1;
            }

            bool timeout_expired(const std::chrono::steady_clock::time_point& start_time) const
            {
                return std::chrono::steady_clock::now() - start_time > MAX_TIMEOUT;
            }

        public:
            void s_lock(const file_mgr::block_id& blk)
            {
                std::unique_lock<std::mutex> lock(mtx);
                auto start = std::chrono::steady_clock::now();

                while (has_x_lock(blk) && !timeout_expired(start))
                    cond.wait_for(lock, MAX_TIMEOUT);

                if (has_x_lock(blk))
                    throw exceptions::lock_abort_exception("x_lock");

                locks[blk] = get_lock_value(blk) + 1;
            }

            void x_lock(const file_mgr::block_id& blk)
            {
                std::unique_lock<std::mutex> lock(mtx);
                auto start = std::chrono::steady_clock::now();
                
                while (has_s_locks(blk) && !timeout_expired(start))
                    cond.wait_for(lock, MAX_TIMEOUT);

                if (has_s_locks(blk))
                    throw exceptions::lock_abort_exception("s_lock");

                locks[blk] = -1;
            }

            void unlock(const file_mgr::block_id& blk)
            {
                std::unique_lock<std::mutex> lock(mtx);
                int value = get_lock_value(blk);

                if (value > 1)
                    locks[blk] = value - 1;
                else
                {
                    locks.erase(blk);
                    cond.notify_all();
                }
            }
    };
}

#endif