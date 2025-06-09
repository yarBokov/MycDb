#ifndef __RECOVERY_MGR_IMPL_H
#define __RECOVERY_MGR_IMPL_H

#include "libs/dbcore/log/log_mgr.h"
#include "libs/dbcore/buffer/buffer_mgr.h"

#include <unordered_set>

namespace dbcore::tx
{
    class transaction;

    class recovery_mgr
    {
        private:
            log_mgr::log_mgr& m_lm;
            buffer_mgr::buffer_mgr& m_bm;
            std::shared_ptr<transaction> m_tx;
            int m_tx_num;

            void do_rollback();
            void do_recover();

        public:
            recovery_mgr() = default;
            recovery_mgr(transaction* tx, int tx_num, log_mgr::log_mgr& lm, buffer_mgr::buffer_mgr& bm);

            void commit();
            void rollback();
            void recover();
            int set_int(buffer_mgr::buffer& buf, int offset, int new_val);
            int set_string(buffer_mgr::buffer& buf, int offset, const std::string& new_val);
    };
}

#endif