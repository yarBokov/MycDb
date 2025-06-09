#include "start_record.h"

namespace dbcore::tx
{
    start_record::start_record(file_mgr::page& p)
    {
        int t_pos = sizeof(int);
        m_tx_num = p.get_int(t_pos);
    }

    log_operation start_record::operation() const { return log_operation::start; }
    int start_record::tx_num() const { return m_tx_num; }
    std::string start_record::to_string() const { return "<START " + std::to_string(m_tx_num) + ">"; }

    int start_record::write_to_log(log_mgr::log_mgr& lm, int tx_number)
    {
        std::vector<char> rec(2 * sizeof(int));
        file_mgr::page p(rec);
        p.set_int(0, log_operation_to_int(log_operation::start));
        p.set_int(sizeof(int), tx_number);
        return lm.append(rec);
    }
}