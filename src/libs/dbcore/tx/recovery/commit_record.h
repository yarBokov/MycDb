#ifndef __TX_COMMIT_RECORD_H
#define __TX_COMMIT_RECORD_H

#include "log_record.h"
#include "libs/dbcore/log/log_mgr.h"

namespace dbcore::tx
{
    class commit_record : public log_record
    {
        private: 
            int m_tx_num;

        public:
            commit_record() : m_tx_num(-1) {}
            explicit commit_record(file_mgr::page& p);
        
            log_operation operation() const override;
            int tx_num() const override;
            void undo(std::shared_ptr<transaction> tx) override;
            std::string to_string() const override;

            static int write_to_log(log_mgr::log_mgr& lm, int tx_number);
    };
}

#endif