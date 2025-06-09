#ifndef __INDEX_IDX_JOIN_PLAN_H
#define __INDEX_IDX_JOIN_PLAN_H

#include "libs/dbcore/query/plan/i_plan.h"
#include "libs/dbcore/metadata/index_info.h"

using namespace dbcore::query::plan;

namespace dbcore::index
{
    class index_join_plan : public i_plan
    {
        private:
            i_plan* m_lhs_plan;
            std::shared_ptr<i_plan> m_rhs_plan;
            std::shared_ptr<metadata::index_info> m_idx_stats;
            std::string m_join_field;
            std::shared_ptr<record::schema> m_sch;

        public:
            index_join_plan(i_plan* lhs_plan, std::shared_ptr<i_plan> rhs_plan, 
                std::shared_ptr<metadata::index_info> idx_stats, const std::string& field);
            std::shared_ptr<scan::i_scan> open() override;
            std::size_t blocks_accessed() const override;
            std::size_t records() const override;
            std::size_t distinct_values(const std::string& fldname) const override;
            std::shared_ptr<record::schema> schema() override;
    };
}

#endif