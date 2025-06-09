#include "log_record.h"

#include "checkpoint_record.h"
#include "commit_record.h"
#include "rollback_record.h"
#include "set_int_record.h"
#include "set_str_record.h"
#include "start_record.h"

namespace dbcore::tx
{
    std::unique_ptr<log_record> log_record::create_log_record(const std::vector<char>& bytes)
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
}