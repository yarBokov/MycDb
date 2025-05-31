#ifndef __TX_TRANSACTION_H
#define __TX_TRANSACTION_H

#include <string>
#include <mutex>

#include "libs/dbcore/file/file_mgr.h"
#include "libs/dbcore/buffer/buffer_mgr.h"
#include "libs/dbcore/log/log_mgr.h"
#include "buffer_list.h"
#include "recovery/recovery_mgr.h"
#include "concurrency/concurrency_mgr.h"

namespace dbcore::tx
{
    class transaction
    {
        private:
            static int next_tx;
            static const int npos = -1;
            static std::mutex tx_num_mtx;
            
            recovery_mgr recovery_mgr;
            concurrency_mgr concurrency_mgr;
            buffer_mgr::buffer_mgr& bm;
            file_mgr::file_mgr& fm;
            int tx_num;
            buffer_list buffers;
            
            static int next_tx_num()
            {
                std::lock_guard<std::mutex> lock(tx_num_mtx);
                return ++next_tx;
            }

        public:
            transaction() = default;
            transaction(file_mgr::file_mgr& fm, log_mgr::log_mgr& lm, buffer_mgr::buffer_mgr& bm);
    
            void commit();
            void rollback();
            void recover();
            
            void pin(const file_mgr::block_id& blk);
            void unpin(const file_mgr::block_id& blk);
            
            int get_int(const file_mgr::block_id& blk, int offset);
            std::string get_str(const file_mgr::block_id& blk, int offset);
            
            void set_int(const file_mgr::block_id& blk, int offset, int value, bool ok_to_log);
            void set_str(const file_mgr::block_id& blk, int offset, const std::string& value, bool ok_to_log);
            
            int size(const std::string& filename);
            std::unique_ptr<file_mgr::block_id> append(const std::string& filename);
            
            int block_size() const;
            int available_buffers_count() const;
    };
}

#endif