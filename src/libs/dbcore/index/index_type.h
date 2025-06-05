#ifndef __INDEX_IDX_TYPE_H
#define __INDEX_IDX_TYPE_H

namespace dbcore::index
{
    enum class index_type
    {
        hash_index = 0,
        bitmap_index,
        binarytree_index
    };


}

#endif