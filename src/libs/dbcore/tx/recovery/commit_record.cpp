#include "commit_record.h"

namespace dbcore::tx
{
    commit_record::commit_record() : m_tx_num(-1) {}
    commit_record::commit_record(file_mgr::page& p)
    {
        int t_pos = sizeof(int);
        m_tx_num = p.get_int(t_pos);
    }

    log_operation commit_record::operation() const { return log_operation::commit; }
    int commit_record::tx_num() const { return m_tx_num; }
    std::string commit_record::to_string() const { return "<COMMIT " + std::to_string(m_tx_num) + ">"; }

    int commit_record::write_to_log(log_mgr::log_mgr& lm, int tx_number)
    {
        std::vector<char> rec(2 * sizeof(int));
        file_mgr::page p(rec);
        p.set_int(0, log_operation_to_int(log_operation::commit));
        p.set_int(sizeof(int), tx_number);
        return lm.append(rec);
    }
}