#include "rollback_record.h"

namespace dbcore::tx
{
    rollback_record::rollback_record() : m_tx_num(-1) {}

    rollback_record::rollback_record(file_mgr::page& p)
    {
        int t_pos = sizeof(int);
        m_tx_num = p.get_int(t_pos);
    }

    log_operation  rollback_record::operation() const { return log_operation::rollback; }
    int  rollback_record::tx_num() const { return m_tx_num; }
    std::string  rollback_record::to_string() const { return "<ROLLBACK " + std::to_string(m_tx_num) + ">"; }

    int  rollback_record::write_to_log(log_mgr::log_mgr& lm, int tx_number)
    {
        std::vector<char> rec(2 * sizeof(int));
        file_mgr::page p(rec);
        p.set_int(0, log_operation_to_int(log_operation::rollback));
        p.set_int(sizeof(int), tx_number);
        return lm.append(rec);
    }
}