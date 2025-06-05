#ifndef __INDEX_BTREE_IDX_DIR_ENTRY_H
#define __INDEX_BTREE_IDX_DIR_ENTRY_H

#include "libs/dbcore/query/constant.h"

namespace dbcore::index
{
    class dir_entry
    {
        private:
            query::constant m_dataval;
            int m_blk_num;

        public:
            dir_entry() = default;

            dir_entry(const query::constant& dataval, int blk_num)
                : m_dataval(dataval), m_blk_num(blk_num)
            {}

            query::constant data_val() const
            {
                return m_dataval;
            }

            int block_number() const
            {
                return m_blk_num;
            }
    };
}

#endif