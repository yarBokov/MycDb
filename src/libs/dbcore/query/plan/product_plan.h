#ifndef __QUERY_PRODUCT_PLAN_H
#define __QUERY_PRODUCT_PLAN_H

#include "i_plan.h"

#include "libs/dbcore/scan/product_scan.h"

namespace dbcore::query::plan
{
    class product_plan;

    product_plan* get_best_plan(std::unique_ptr<i_plan> plan_lhs, std::unique_ptr<i_plan> plan_rhs)
    {
        auto prod1 = new product_plan(std::move(plan_lhs), std::move(plan_rhs));
        auto prod2 = new product_plan(std::move(plan_rhs), std::move(plan_lhs));
        int blocks1 = prod1->blocks_accessed();
        int blocks2 = prod2->blocks_accessed();
        auto best_plan = (blocks1 < blocks2) ? prod1 : prod2;
        if (best_plan == prod1)
            delete prod2;
        else
            delete prod1;
        return best_plan;
    }

    class product_plan : public i_plan
    {
        private:
            std::unique_ptr<i_plan> m_plan_lhs;
            std::unique_ptr<i_plan> m_plan_rhs;
            std::shared_ptr<record::schema> m_sch;

        public:
            explicit product_plan(std::unique_ptr<i_plan> plan_lhs, std::unique_ptr<i_plan> plan_rhs)
                : m_plan_lhs(std::move(plan_lhs)), m_plan_rhs(std::move(plan_rhs))
            {
                m_sch->add_all(*m_plan_lhs->schema());
                m_sch->add_all(*m_plan_rhs->schema());
            }

            std::shared_ptr<scan::i_scan> open() override
            {
                auto s_lhs = m_plan_lhs->open();
                auto s_rhs = m_plan_rhs->open();
                return std::make_shared<scan::product_scan>(s_lhs, s_rhs);
            }

            std::size_t blocks_accessed() const override
            {
                return m_plan_lhs->blocks_accessed() 
                    + (m_plan_lhs->records() * m_plan_rhs->blocks_accessed());
            }

            std::size_t records() const override
            {
                return m_plan_lhs->records() * m_plan_rhs->records();
            }

            std::size_t distinct_values(const std::string& fldname) const override
            {
                if (m_plan_lhs->schema()->has_field(fldname))
                    return m_plan_lhs->distinct_values(fldname);
                else if (m_plan_rhs->schema()->has_field(fldname))
                    return m_plan_rhs->distinct_values(fldname);
            }

            std::shared_ptr<record::schema> schema() override
            {
                return m_sch;
            }
    };
}

#endif