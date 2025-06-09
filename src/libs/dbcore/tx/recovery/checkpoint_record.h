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

            log_operation operation() const override;
            int tx_num() const override;
            void undo(std::shared_ptr<transaction> tx) override { }
            std::string to_string() const override;

            static int write_to_log(log_mgr::log_mgr& lm);
    };
}

#endif