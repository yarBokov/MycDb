#ifndef __TX_BUFFER_LIST_H
#define __TX_BUFFER_LIST_H

#include <unordered_map>
#include <vector>
#include <memory>

#include "libs/dbcore/file/page/block_id.h"
#include "libs/dbcore/buffer/buffer_mgr.h"

namespace dbcore::tx
{
    class buffer_list
    {
        private:
            std::unordered_map<file_mgr::block_id, 
                std::shared_ptr<buffer_mgr::buffer>, file_mgr::block_id::hash> buffers;
                
            std::vector<file_mgr::block_id> pins;
            buffer_mgr::buffer_mgr& bm;

        public:
            buffer_list(buffer_mgr::buffer_mgr& bm)
                : bm(bm)
            {}

            std::shared_ptr<buffer_mgr::buffer> get_buffer(const file_mgr::block_id& blk)
            {
                auto it = buffers.find(blk);
                return (it != buffers.end()) ? it->second : nullptr;
            }

            void pin(const file_mgr::block_id& blk)
            {
                auto& pinned_buf = bm.pin(blk);
                buffers[blk] = std::make_shared<buffer_mgr::buffer>(pinned_buf);
                pins.push_back(blk);
            }

            void unpin(const file_mgr::block_id& blk)
            {
                auto buf = get_buffer(blk);
                if (buf)
                {
                    bm.unpin(*buf);
                    pins.erase(std::remove(pins.begin(), pins.end(), blk), pins.end());
                    if (std::find(pins.begin(), pins.end(), blk) == pins.end())
                        buffers.erase(blk);
                }
            }

            void unpin_all()
            {
                for (auto& blk : pins)
                {
                    auto buf = get_buffer(blk);
                    if (buf)
                        bm.unpin(*buf);
                }
                buffers.clear();
                pins.clear();
            }
    };
}

#endif