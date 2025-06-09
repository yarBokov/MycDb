#ifndef __MATERIALIZE_SORT_PLAN_H
#define __MATERIALIZE_SORT_PLAN_H

#include "libs/dbcore/query/plan/i_plan.h"
#include "libs/dbcore/tx/transaction.h"
#include "record_comparator.h"
#include "sort_scan.h"
#include "materialize_plan.h"

using namespace dbcore::query::plan;

namespace dbcore::materialize
{
    class sort_plan : public i_plan
    {
        private:
            std::shared_ptr<tx::transaction> m_tx;
            std::shared_ptr<i_plan> m_plan;
            record::schema m_sch;
            std::shared_ptr<record_comparator> m_rec_comp;

            bool copy(std::shared_ptr<scan::i_scan> src, std::shared_ptr<scan::i_update_scan> dest)
            {
                dest->insert();
                for (const auto& fldname : m_sch.fields())
                    dest->set_val(fldname, src->get_val(fldname));
                return src->next();
            }

            std::shared_ptr<temp_table> merge_two_runs(
                std::shared_ptr<temp_table> plan1, std::shared_ptr<temp_table> plan2)
            {
                auto src1 = plan1->open();
                auto src2 = plan2->open();
                auto result = std::make_shared<temp_table>(m_tx, m_sch);
                auto dest = result->open();

                bool has_more1 = src1->next();
                bool has_more2 = src2->next();

                while (has_more1 && has_more2)
                {
                    if (!m_rec_comp->compare(src1, src2))
                        has_more1 = copy(src1, dest);
                    else
                        has_more2 = copy(src2, dest);
                }

                while (has_more1)
                    has_more1 = copy(src1, dest);
                
                while (has_more2)
                    has_more2 = copy(src2, dest);

                src1->close();
                src2->close();
                dest->close();
                return result;
            }

            std::vector<std::shared_ptr<temp_table>> split_into_runs(std::shared_ptr<scan::i_scan> src)
            {
                std::vector<std::shared_ptr<temp_table>> temps;
                src->before_first();
                if (!src->next())
                    return temps;
                
                auto curr_temp = std::make_shared<temp_table>(m_tx, m_sch);
                temps.push_back(curr_temp);
                auto curr_scan = curr_temp->open();

                while (copy(src, curr_scan))
                {
                    if (!m_rec_comp->compare(src, curr_scan))
                    {
                        curr_scan->close();
                        curr_temp = std::make_shared<temp_table>(m_tx, m_sch);
                        temps.push_back(curr_temp);
                        curr_scan = curr_temp->open();
                    }
                }

                curr_scan->close();
                return temps;
            }

            std::vector<std::shared_ptr<temp_table>> do_a_merge_iteration(
                const std::vector<std::shared_ptr<temp_table>>& runs)
            {
                std::vector<std::shared_ptr<temp_table>> result;
                auto remaining_runs = runs;

                while (remaining_runs.size() > 1)
                {
                    auto plan1 = remaining_runs.front();
                    remaining_runs.erase(remaining_runs.begin());
                    auto plan2 = remaining_runs.front();
                    remaining_runs.erase(remaining_runs.begin());
                    result.push_back(merge_two_runs(plan1, plan2));
                }

                if (!remaining_runs.empty())
                    result.push_back(remaining_runs.front());

                return result;
            }
        
        public:
            sort_plan(std::shared_ptr<tx::transaction> tx, std::shared_ptr<i_plan> plan,
                const std::vector<std::string>& sort_fields)
                : m_tx(tx)
                , m_plan(plan)
                , m_sch(plan->schema())
                , m_rec_comp(std::make_shared<record_comparator>(sort_fields))
            {}

            std::shared_ptr<scan::i_scan> open() override
            {
                auto src = m_plan->open();
                auto runs = split_into_runs(src);
                src->close();
                while (runs.size() > 2)
                    runs = do_a_merge_iteration(runs);

                return std::make_shared<sort_scan>(runs, m_rec_comp);
            }

            std::size_t blocks_accessed() const override
            {
                auto mp = std::make_shared<materialize_plan>(m_tx, m_plan);
                return mp->blocks_accessed();
            }

            std::size_t records() const override
            {
                return m_plan->records();
            }

            std::size_t distinct_values(const std::string& fldname) const override
            {
                return m_plan->distinct_values(fldname);
            }

            record::schema schema() override
            {
                return m_sch;
            }
    };
}

#endif