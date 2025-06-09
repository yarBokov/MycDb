#include "temp_table.h"

#include "libs/dbcore/record/table_scan.h"

namespace dbcore::materialize
{
    std::mutex temp_table::m_mtx;
    int temp_table::m_next_tbl_num = 0;

    std::string temp_table::next_table_name()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_next_tbl_num++;
        return "temp" + std::to_string(m_next_tbl_num);
    }

    temp_table::temp_table(std::shared_ptr<tx::transaction> tx, std::shared_ptr<record::schema> sch)
        : m_tx(tx), m_layout(sch)
    {
        m_tblname = next_table_name();
    }

    std::shared_ptr<scan::i_update_scan> temp_table::open()
    {
        return std::make_shared<record::table_scan>(m_tx, m_tblname, m_layout);
    }

    std::string temp_table::get_table_name() const
    {
        return m_tblname;
    }

    const record::layout temp_table::get_layout() const
    {
        return m_layout;
    }
}