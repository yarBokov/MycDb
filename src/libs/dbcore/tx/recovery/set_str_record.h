#ifndef __TX_SET_STRING_RECORD_H
#define __TX_SET_STRING_RECORD_H

#include "log_record.h"
#include "libs/dbcore/log/log_mgr.h"

namespace dbcore::tx
{
    class set_str_record : public log_record
    {
        private: 
            int m_tx_num;
            int offset;
            std::string value;
            std::unique_ptr<file_mgr::block_id> blk;

        public:
            set_str_record() = default;
            explicit set_str_record(file_mgr::page& p);
        
            log_operation operation() const override;
            int tx_num() const override;

            void undo(std::shared_ptr<transaction> tx) override;

            std::string to_string() const override;

            static int write_to_log(log_mgr::log_mgr& lm, int tx_num, 
                const file_mgr::block_id& blk, int offset, const std::string& value);
    };
}

#endif