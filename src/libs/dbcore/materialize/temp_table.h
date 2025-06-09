#ifndef __MATERIALIZE_TEMP_TABLE_H
#define __MATERIALIZE_TEMP_TABLE_H

#include "libs/dbcore/tx/transaction.h"
#include "libs/dbcore/record/schema.h"
#include "libs/dbcore/record/layout.h"
#include "libs/dbcore/scan/i_update_scan.h"

#include <mutex>

namespace dbcore::materialize
{
    class temp_table
    {
        private:
            static std::mutex m_mtx;
            static int m_next_tbl_num;

            std::shared_ptr<tx::transaction> m_tx;
            std::string m_tblname;
            record::layout m_layout;

            static std::string next_table_name();

        public:
            temp_table(std::shared_ptr<tx::transaction> tx, std::shared_ptr<record::schema> sch);

            std::shared_ptr<scan::i_update_scan> open();
            std::string get_table_name() const;
            const record::layout get_layout() const;
    };
}

#endif