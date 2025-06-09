#ifndef __MATERIALIZE_GROUP_BY_PLAN_H
#define __MATERIALIZE_GROUP_BY_PLAN_H

#include "group_value.h"
#include "fn/i_aggregate_fn.h"
#include "group_by_scan.h"
#include "sort_plan.h"
#include "libs/dbcore/query/plan/i_plan.h"

using namespace dbcore::query::plan;

namespace dbcore::materialize
{
    class group_by_plan : public i_plan
    {
        private:
            std::shared_ptr<i_plan> m_plan;
            std::vector<std::string> m_group_fields;
            std::vector<std::shared_ptr<i_aggregate_fn>> m_aggr_funcs;
            std::shared_ptr<record::schema> m_sch;

        public:
            group_by_plan(std::shared_ptr<tx::transaction> tx, std::shared_ptr<i_plan> plan,
                          const std::vector<std::string>& group_fields,
                          const std::vector<std::shared_ptr<i_aggregate_fn>> aggr_funcs)
                : m_plan(std::make_shared<sort_plan>(tx, plan, group_fields))
                , m_group_fields(group_fields)
                , m_aggr_funcs(aggr_funcs)
            {
                for (const auto& fldname : m_group_fields)
                    m_sch->add(fldname, *m_plan->schema());

                for (auto& fn : m_aggr_funcs)
                    m_sch->add_int_field(fn->field_name());
            }

            std::shared_ptr<scan::i_scan> open() override
            {
                auto s = m_plan->open();
                return std::make_shared<group_by_scan>(s, m_group_fields, m_aggr_funcs);
            }

            std::size_t blocks_accessed() const override
            {
                return m_plan->blocks_accessed();
            }

            std::size_t records() const override
            {
                int num_groups = 1;
                for (const auto& fldname : m_group_fields)
                    num_groups *= m_plan->distinct_values(fldname);

                return num_groups;
            }

            std::size_t distinct_values(const std::string& fldname) const override
            {
                if (m_plan->schema()->has_field(fldname))
                    return m_plan->distinct_values(fldname);
                return records();
            }

            std::shared_ptr<record::schema> schema() override
            {
                return m_sch;
            }
    };
}

#endif