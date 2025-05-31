#ifndef __QUERY_SELECT_PLAN_H
#define __QUERY_SELECT_PLAN_H

#include "i_plan.h"

#include "libs/dbcore/query/predicate.h"
#include "libs/dbcore/scan/select_scan.h"

#include <cmath>

namespace dbcore::query::plan
{
    class select_plan : public i_plan
    {
        private:
            std::unique_ptr<i_plan> m_plan;
            query::predicate m_pred;

        public:
            explicit select_plan(std::unique_ptr<i_plan> plan, const query::predicate& pred)
                : m_plan(std::move(plan)), m_pred(pred)
            {}

            std::unique_ptr<scan::i_scan> open() override
            {
                auto s = m_plan->open();
                return std::make_unique<scan::select_scan>(s, m_pred);
            }

            std::size_t blocks_accessed() const override
            {
                return m_plan->blocks_accessed();
            }

            std::size_t records() const override
            {
                return m_plan->records() / m_pred.calculate_reduction(*m_plan);
            }

            std::size_t distinct_values(const std::string& fldname) const override
            {
                if (m_pred.equates_with_constant(fldname).to_string() != "0")
                    return 1;
                else
                {
                    std::string fldname_other = m_pred.equates_with_field(fldname);
                    if (!fldname_other.empty())
                    {
                        return std::min(m_plan->distinct_values(fldname), m_plan->distinct_values(fldname_other));
                    }
                    else
                        return m_plan->distinct_values(fldname);
                }
            }

            record::schema schema() override
            {
                return m_plan->schema();
            }
    };
}

#endif