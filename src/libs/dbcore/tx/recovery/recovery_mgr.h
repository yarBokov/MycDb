#ifndef __RECOVERY_MGR_IMPL_H
#define __RECOVERY_MGR_IMPL_H

#include "libs/dbcore/log/log_mgr.h"
#include "libs/dbcore/buffer/buffer_mgr.h"

#include "start_record.h"
#include "commit_record.h"
#include "rollback_record.h"
#include "set_int_record.h"
#include "set_str_record.h"
#include "checkpoint_record.h"
#include "log_record.h"

#include <unordered_set>

namespace dbcore::tx
{
    class recovery_mgr
    {
        private:
            log_mgr::log_mgr& lm;
            buffer_mgr::buffer_mgr& bm;
            transaction& tx;
            int tx_num;

            void do_rollback()
            {
                auto log_iter = lm.iterator();
                while (log_iter.has_next())
                {
                    auto bytes = log_iter.next();
                    auto rec = log_record::create_log_record(bytes);
                    if (rec->tx_num() == tx_num)
                    {
                        if (rec->operation() == log_operation::start)
                            return;
                        rec->undo(tx);
                    }
                }
            }

            void do_recover()
            {
                std::unordered_set<int> finished_txs;
                auto log_iter = lm.iterator();
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
                        rec->undo(tx);
                }
            }

        public:
            recovery_mgr() = default;
            recovery_mgr(transaction& tx, int tx_num, log_mgr::log_mgr& lm, buffer_mgr::buffer_mgr& bm)
                : tx(tx), tx_num(tx_num), lm(lm), bm(bm)
            {
                start_record::write_to_log(this->lm, this->tx_num);
            }

            void commit()
            {
                bm.flush_all(tx_num);
                int lsn = commit_record::write_to_log(lm, tx_num);
                lm.flush(lsn);
            }

            void rollback()
            {
                do_rollback();
                bm.flush_all(tx_num);
                int lsn = rollback_record::write_to_log(lm, tx_num);
                lm.flush(lsn);
            }

            void recover()
            {
                do_recover();
                bm.flush_all(tx_num);
                int lsn = checkpoint_record::write_to_log(lm);
                lm.flush(lsn);
            }

            int set_int(buffer_mgr::buffer& buf, int offset, int new_val)
            {
                int old_val = buf.contents().get_int(offset);
                auto blk = buf.block();
                return set_int_record::write_to_log(lm, tx_num, *blk ,offset, old_val);
            }

            int set_string(buffer_mgr::buffer& buf, int offset, const std::string& new_val)
            {
                std::string old_val = buf.contents().get_string(offset);
                auto blk = buf.block();
                return set_str_record::write_to_log(lm, tx_num, *blk ,offset, old_val);
            }
    };
}

#endif