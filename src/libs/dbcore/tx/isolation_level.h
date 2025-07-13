#ifndef __TX_ISOLATION_LEVELS_H
#define __TX_ISOLATION_LEVELS_H

namespace tx
{
    enum class isolation_level
    {
        read_uncommited = 1,
        read_commited,
        repeatable_read,
        serializable
    };
}

#endif