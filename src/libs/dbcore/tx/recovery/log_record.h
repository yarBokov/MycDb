#ifndef __TX_LOG_RECORD_H
#define __TX_LOG_RECORD_H

#include "libs/dbcore/tx/log_ops.h"

#include <string>
#include <memory>
#include <vector>

#include "libs/dbcore/file/page/page.h"
#include "libs/dbcore/tx/transaction.h"

namespace dbcore::tx
{
    class transaction;
    
    class log_record
    {
        public:
            virtual ~log_record() = default;
            virtual log_operation operation() const = 0;
            virtual int tx_num() const = 0;
            virtual void undo(std::shared_ptr<transaction> tx) = 0;
            virtual std::string to_string() const = 0;

            static std::unique_ptr<log_record> create_log_record(const std::vector<char>& bytes);
    };
}

#endif