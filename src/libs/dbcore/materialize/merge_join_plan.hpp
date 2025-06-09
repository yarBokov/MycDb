#ifndef __MATERIALIZE_MERGE_JOIN_PLAN_H
#define __MATERIALIZE_MERGE_JOIN_PLAN_H

#include "sort_plan.h"
#include "merge_join_scan.hpp"

namespace dbcore::materialize
{
    class merge_join_plan : public i_plan
    {
        private:
            std::shared_ptr<i_plan> m_lhs_plan;
            std::shared_ptr<i_plan> m_rhs_plan;
            std::string m_fldname1;
            std::string m_fldname2;
            std::shared_ptr<record::schema> m_sch;

        public:
            merge_join_plan(std::shared_ptr<tx::transaction> tx,
                            std::shared_ptr<i_plan> lhs_plan,
                            std::shared_ptr<i_plan> rhs_plan,
                            const std::string& fldname1,
                            const std::string& fldname2)
                : m_fldname1(fldname1), m_fldname2(fldname2)
            {
                std::vector<std::string> sortlist1 = {m_fldname1};
                m_lhs_plan = std::make_shared<sort_plan>(tx, lhs_plan, sortlist1);

                std::vector<std::string> sortlist2 = {m_fldname2};
                m_rhs_plan = std::make_shared<sort_plan>(tx, rhs_plan, sortlist2);

                m_sch->add_all(*m_lhs_plan->schema());
                m_sch->add_all(*m_rhs_plan->schema());
            }

            std::shared_ptr<scan::i_scan> open() override
            {
                auto s1 = m_lhs_plan->open();
                auto s2 = std::dynamic_pointer_cast<sort_scan>(m_rhs_plan->open());
                return std::make_shared<merge_join_scan>(s1, s2, m_fldname1, m_fldname2);
            }

            std::size_t blocks_accessed() const override
            {
                return m_lhs_plan->blocks_accessed() + m_rhs_plan->blocks_accessed();
            }

            std::size_t records() const override
            {
                int max_vals = std::max(m_lhs_plan->distinct_values(m_fldname1), m_rhs_plan->distinct_values(m_fldname2));
                return (m_lhs_plan->records() * m_rhs_plan->records()) / max_vals;
            }

            std::size_t distinct_values(const std::string& fldname) const override
            {
                if (m_lhs_plan->schema()->has_field(fldname))
                    return m_lhs_plan->distinct_values(fldname);

                return m_rhs_plan->distinct_values(fldname);
            }

            std::shared_ptr<record::schema> schema() override
            {
                return m_sch;
            }
    };
}

#endif