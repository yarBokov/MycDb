#ifndef __TX_COMMIT_RECORD_H
#define __TX_COMMIT_RECORD_H

#include "log_record.h"
#include "libs/dbcore/log/log_mgr.h"

namespace dbcore::tx
{
    class commit_record : public log_record
    {
        private: int tx_num;
        public:
            commit_record() : tx_num(-1) {}
            explicit commit_record(file_mgr::page& p)
            {
                int t_pos = sizeof(int);
                tx_num = p.get_int(t_pos);
            }
        
            log_operation operation() const override { return log_operation::commit; }
            int tx_num() const override { return tx_num; }
            void undo(transaction& tx) override { }
            std::string to_string() const override { return "<COMMIT " + std::to_string(tx_num) + ">"; }

            static int write_to_log(log_mgr::log_mgr& lm, int tx_num)
            {
                std::vector<char> rec(2 * sizeof(int));
                file_mgr::page p(rec);
                p.set_int(0, log_operation_to_int(log_operation::commit));
                p.set_int(sizeof(int), tx_num);
                return lm.append(rec);
            }
    };
}

#endif