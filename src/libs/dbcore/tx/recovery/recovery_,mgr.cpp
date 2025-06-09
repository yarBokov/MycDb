#include "recovery_mgr.h"

#include "start_record.h"
#include "commit_record.h"
#include "rollback_record.h"
#include "set_int_record.h"
#include "set_str_record.h"
#include "checkpoint_record.h"

#include "libs/dbcore/tx/transaction.h"

namespace dbcore::tx
{
    void recovery_mgr::do_rollback()
    {
        auto log_iter = m_lm.iterator();
        while (log_iter.has_next())
        {
            auto bytes = log_iter.next();
            auto rec = log_record::create_log_record(bytes);
            if (rec->tx_num() == m_tx_num)
            {
                if (rec->operation() == log_operation::start)
                    return;
                rec->undo(m_tx);
            }
        }
    }

    void recovery_mgr::do_recover()
    {
        std::unordered_set<int> finished_txs;
        auto log_iter = m_lm.iterator();
        while (log_iter.has_next())
        {
            auto bytes = log_iter.next();
            auto rec = log_record::create_log_record(bytes);
            if (rec->operation() == log_operation::checkpoint)
                return;
            if (rec->operation() == log_operation::commit 
                || rec->operation() == log_operation::rollback)
                finished_txs.insert(rec->tx_num());
            else if (finished_txs.find(rec->tx_num()) == finished_txs.end())
                rec->undo(m_tx);
        }
    }

    recovery_mgr::recovery_mgr(transaction* tx, int tx_num, log_mgr::log_mgr& lm, buffer_mgr::buffer_mgr& bm)
        : m_tx(tx), m_tx_num(tx_num), m_lm(lm), m_bm(bm)
    {
        start_record::write_to_log(m_lm, m_tx_num);
    }

    void recovery_mgr::commit()
    {
        m_bm.flush_all(m_tx_num);
        int lsn = commit_record::write_to_log(m_lm, m_tx_num);
        m_lm.flush(lsn);
    }

    void recovery_mgr::rollback()
    {
        do_rollback();
        m_bm.flush_all(m_tx_num);
        int lsn = rollback_record::write_to_log(m_lm, m_tx_num);
        m_lm.flush(lsn);
    }

    void recovery_mgr::recover()
    {
        do_recover();
        m_bm.flush_all(m_tx_num);
        int lsn = checkpoint_record::write_to_log(m_lm);
        m_lm.flush(lsn);
    }

    int recovery_mgr::set_int(buffer_mgr::buffer& buf, int offset, int new_val)
    {
        int old_val = buf.contents().get_int(offset);
        auto blk = buf.block();
        return set_int_record::write_to_log(m_lm, m_tx_num, *blk , offset, old_val);
    }

    int recovery_mgr::set_string(buffer_mgr::buffer& buf, int offset, const std::string& new_val)
    {
        std::string old_val = buf.contents().get_string(offset);
        auto blk = buf.block();
        return set_str_record::write_to_log(m_lm, m_tx_num, *blk, offset, old_val);
    }
}