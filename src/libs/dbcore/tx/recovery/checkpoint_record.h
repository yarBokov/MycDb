#ifndef __TX_CHECKPOINT_RECORD_H
#define __TX_CHECKPOINT_RECORD_H

#include "log_record.h"
#include "libs/dbcore/log/log_mgr.h"

namespace dbcore::tx
{
    class checkpoint_record : public log_record
    {
        public:
            checkpoint_record() = default;

            log_operation operation() const override { return log_operation::checkpoint; }
            int tx_num() const override { return -1; }
            void undo(transaction& tx) override { }
            std::string to_string() const override { return "<CHECKPOINT>"; }

            static int write_to_log(log_mgr::log_mgr& lm)
            {
                std::vector<char> rec(sizeof(int));
                file_mgr::page p(rec);
                p.set_int(0, log_operation_to_int(log_operation::checkpoint));
                return lm.append(rec);
            }
    };
}

#endif