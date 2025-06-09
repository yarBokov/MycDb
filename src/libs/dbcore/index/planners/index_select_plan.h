#ifndef __INDEX_IDX_SELECT_PLAN_H
#define __INDEX_IDX_SELECT_PLAN_H

#include "libs/dbcore/query/plan/i_plan.h"
#include "libs/dbcore/metadata/index_info.h"

using namespace dbcore::query::plan;

namespace dbcore::index
{
    class index_select_plan : public i_plan
    {
        private:
            std::shared_ptr<i_plan> m_plan;
            std::shared_ptr<metadata::index_info> m_idx_stats;
            query::constant m_val;

        public:
            index_select_plan(std::shared_ptr<i_plan> p, std::shared_ptr<metadata::index_info> idx_stats, const query::constant& val);
            std::shared_ptr<scan::i_scan> open() override;
            std::size_t blocks_accessed() const override;
            std::size_t records() const override;
            std::size_t distinct_values(const std::string& fldname) const override;
            std::shared_ptr<record::schema> schema() override;
    };
}

#endif