#include "index_join_plan.h"

#include "libs/dbcore/record/table_scan.h"
#include "libs/dbcore/index/scan/index_join_scan.h"

namespace dbcore::index
{
    index_join_plan::index_join_plan(i_plan* lhs_plan, std::shared_ptr<i_plan> rhs_plan, 
        std::shared_ptr<metadata::index_info> idx_stats, const std::string& field)
        : m_lhs_plan(lhs_plan), m_rhs_plan(std::move(rhs_plan)), m_idx_stats(std::move(idx_stats)), m_join_field(field)
    {
        m_sch.add_all(m_lhs_plan->schema());
        m_sch.add_all(m_rhs_plan->schema());
    }

    std::shared_ptr<scan::i_scan> index_join_plan::open()
    {
        auto sc = m_lhs_plan->open();
        auto ts = dynamic_cast<record::table_scan*>(m_rhs_plan->open().get());
        if (!ts)
            throw std::runtime_error("Right-hand plan for index_join_plan must be of type 'table_scan'");
        auto idx = m_idx_stats->open();
        return std::dynamic_pointer_cast<scan::i_scan>(std::make_shared<index_join_scan>(sc,
            idx, m_join_field, std::shared_ptr<record::table_scan>(ts)));
    }

    std::size_t index_join_plan::blocks_accessed() const
    {
        return m_lhs_plan->blocks_accessed() + 
            (m_lhs_plan->records() * m_idx_stats->blocks_accessed()) + records();
    }

    std::size_t index_join_plan::records() const
    {
        return m_lhs_plan->records() * m_idx_stats->records_output();
    }

    std::size_t index_join_plan::distinct_values(const std::string& fldname) const
    {
        if (m_lhs_plan->schema().has_field(fldname))
            return m_lhs_plan->distinct_values(fldname);
        else
            return m_rhs_plan->distinct_values(fldname);
    }

    record::schema index_join_plan::schema()
    {
        return m_sch;
    }
}