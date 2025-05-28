#ifndef __LOG_ITERATOR_H
#define __LOG_ITERATOR_H

#include "libs/dbcore/file/file_mgr.h"

using namespace dbcore::file_mgr;

namespace dbcore::log_mgr
{
    class log_iterator
    {
        private:
            using f_m = dbcore::file_mgr::file_mgr;
            using blk_id = dbcore::file_mgr::block_id;
            using page = dbcore::file_mgr::page;

            std::shared_ptr<f_m> fm;
            blk_id block;
            std::unique_ptr<page> p;
            int cur_pos;
            int boundary;

            void move_to_block(const blk_id& blk)
            {
                fm->read(blk, *p);
                boundary = p->get_int(0);
                cur_pos = boundary;
            }
        
        public:
            log_iterator() = delete;
            ~log_iterator() = default;

            log_iterator(std::shared_ptr<f_m> fm_ptr, const blk_id& blk)
                : fm(std::move(fm_ptr))
                , block(blk)
                , cur_pos{0}
                , boundary{0}
            {
                std::vector<char> buf(fm->get_block_size(), 0);
                p = std::make_unique<page>(buf);
                move_to_block(block);
            }

            bool has_next() const
            {
                return cur_pos < fm->get_block_size() || block.get_block_number() > 0;
            }

            std::vector<char> next()
            {
                if (cur_pos == fm->get_block_size())
                {
                    block = blk_id(block.get_filename(), block.get_block_number() - 1);
                    move_to_block(block);
                }

                auto rec = p->get_bytes(cur_pos);
                cur_pos += static_cast<int>(sizeof(int)) + static_cast<int>(rec.size());
                return rec;
            }
    };
}

#endif