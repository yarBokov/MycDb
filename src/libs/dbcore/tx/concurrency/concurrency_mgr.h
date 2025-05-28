#ifndef __CONCURRENCY_MGR_IMPL_H
#define __CONCURRENCY_MGR_IMPL_H

#include "lock_table.h"

namespace dbcore::tx
{
    class concurrency_mgr
    {
        private:
            static lock_table lock_tbl;
            std::unordered_map<file_mgr::block_id, char, file_mgr::block_id::hash> locks;
            mutable std::mutex mtx;

            bool has_x_lock(const file_mgr::block_id& blk) const
            {
                std::lock_guard<std::mutex> lock(mtx);
                auto it = locks.find(blk);
                return it != locks.end() && it->second == 'X';
            }

        public:
            void s_lock(const file_mgr::block_id& blk)
            {
                std::lock_guard<std::mutex> lock(mtx);
                if (locks.find(blk) == locks.end())
                {
                    lock_tbl.s_lock(blk);
                    locks[blk] = 'S';
                }
            }

            void x_lock(const file_mgr::block_id& blk)
            {
                std::lock_guard<std::mutex> lock(mtx);
                if (!has_x_lock(blk))
                {
                    if (locks.find(blk) == locks.end())
                        lock_tbl.s_lock(blk);

                    lock_tbl.x_lock(blk);
                    locks[blk] = 'X';
                }
            }

            void release()
            {
                std::lock_guard<std::mutex> lock(mtx);
                for (auto& pair : locks)
                    lock_tbl.unlock(pair.first);

                locks.clear();
            }
    };
}

#endif