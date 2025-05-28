#ifndef __TX_OPERATIONS_ENUM_H
#define __TX_OPERATIONS_ENUM_H

namespace dbcore::tx
{
    enum class log_operation
    {
        checkpoint,
        start,
        commit,
        rollback,
        set_int,
        set_str
    };

    int log_operation_to_int(log_operation log_op)
    {
        return (static_cast<int>(log_op) / sizeof(int)) * sizeof(int);
    }
}

#endif