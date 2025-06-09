#include "index_select_plan.h"

#include "libs/dbcore/index/scan/index_select_scan.h"

namespace dbcore::index
{
    index_select_plan::index_select_plan(std::shared_ptr<i_plan> plan, std::shared_ptr<metadata::index_info> idx_stats, const query::constant& val)
        : m_plan(plan), m_idx_stats(idx_stats), m_val(val)
    {}

    std::shared_ptr<scan::i_scan> index_select_plan::open()
    {
        auto ts = dynamic_cast<record::table_scan*>(m_plan->open().get());
        if (!ts)
            throw std::runtime_error("Plan for 'index_select_plan' must be a table_scan");

        auto idx = m_idx_stats->open();
        return std::make_shared<index_select_scan>(std::shared_ptr<record::table_scan>(ts), idx, m_val);
    }

    std::size_t index_select_plan::blocks_accessed() const
    {
        return m_idx_stats->blocks_accessed() + records();
    }

    std::size_t index_select_plan::records() const
    {
        return m_idx_stats->records_output();
    }

    std::size_t index_select_plan::distinct_values(const std::string& fldname) const
    {
        return m_idx_stats->distinct_values(fldname);
    }

    std::shared_ptr<record::schema> index_select_plan::schema()
    {
        return m_plan->schema();
    }
}