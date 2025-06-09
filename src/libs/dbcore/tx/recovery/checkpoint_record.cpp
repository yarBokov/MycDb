#include "checkpoint_record.h"

namespace dbcore::tx
{
    log_operation checkpoint_record::operation() const 
    { 
        return log_operation::checkpoint; 
    }
    int checkpoint_record::tx_num() const
    { 
        return -1; 
    }
    void checkpoint_record::undo(std::shared_ptr<transaction> tx) { }
    std::string checkpoint_record::to_string() const 
    { 
        return "<CHECKPOINT>"; 
    }

    int checkpoint_record::write_to_log(log_mgr::log_mgr& lm)
    {
        std::vector<char> rec(sizeof(int));
        file_mgr::page p(rec);
        p.set_int(0, log_operation_to_int(log_operation::checkpoint));
        return lm.append(rec);
    }
}