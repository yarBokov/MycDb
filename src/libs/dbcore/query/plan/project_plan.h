#ifndef __QUERY_PROJECT_PLAN_H
#define __QUERY_PROJECT_PLAN_H

#include "i_plan.h"

#include "libs/dbcore/scan/project_scan.h"

#include <vector>

namespace dbcore::query::plan
{
    class project_plan : public i_plan
    {
        private:
            std::unique_ptr<i_plan> m_plan;
            std::shared_ptr<record::schema> m_sch;

        public:
            explicit project_plan(std::unique_ptr<i_plan> plan, const std::vector<std::string>& fieldlist)
                : m_plan(std::move(plan))
            {
                for (const auto& fldname : fieldlist)
                    m_sch->add(fldname, *m_plan->schema());
            }

            std::shared_ptr<scan::i_scan> open() override
            {
                auto s = m_plan->open();
                return std::make_shared<scan::project_scan>(s, m_sch->fields());
            }

            std::size_t blocks_accessed() const override
            {
                return m_plan->blocks_accessed();
            }

            std::size_t records() const override
            {
                return m_plan->records();
            }

            std::size_t distinct_values(const std::string& fldname) const override
            {
                return m_plan->distinct_values(fldname);
            }

            std::shared_ptr<record::schema> schema() override
            {
                return m_plan->schema();
            }
    };
}

#endif