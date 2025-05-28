#ifndef __TX_LOG_RECORD_H
#define __TX_LOG_RECORD_H

#include "libs/dbcore/tx/log_ops.h"

#include <string>
#include <memory>
#include <vector>

#include "libs/dbcore/file/page/page.h"
#include "libs/dbcore/tx/transaction.h"

#include "checkpoint_record.h"
#include "start_record.h"
#include "commit_record.h"
#include "rollback_record.h"
#include "set_int_record.h"
#include "set_str_record.h"

namespace dbcore::tx
{
    class log_record
    {
        public:
            virtual ~log_record() = default;
            virtual log_operation operation() const = 0;
            virtual int tx_num() const = 0;
            virtual void undo(transaction& tx) = 0;
            virtual std::string to_string() const = 0;

            static std::unique_ptr<log_record> create_log_record(const std::vector<char>& bytes)
            {
                file_mgr::page p(bytes);
                switch (static_cast<log_operation>(p.get_int(0)))
                {
                    case log_operation::checkpoint:
                        return std::make_unique<checkpoint_record>();
                    case log_operation::start:
                        return std::make_unique<start_record>(p);
                    case log_operation::commit:
                        return std::make_unique<commit_record>(p);
                    case log_operation::rollback:
                        return std::make_unique<rollback_record>(p);
                    case log_operation::set_int:
                        return std::make_unique<set_int_record>(p);
                    case log_operation::set_str:
                        return std::make_unique<set_str_record>(p);
                    default:
                        return nullptr;
                }
            }
    };
}

#endif