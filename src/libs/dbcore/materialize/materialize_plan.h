#ifndef __MATERIALIZE_PLAN_H
#define __MATERIALIZE_PLAN_H

#include "libs/dbcore/query/plan/i_plan.h"
#include "libs/dbcore/tx/transaction.h"
#include "libs/dbcore/record/layout.h"
#include "temp_table.h"

using namespace dbcore::query::plan;

namespace dbcore::materialize
{
    class materialize_plan : public i_plan
    {
        private:
            std::shared_ptr<i_plan> m_src_plan;
            std::shared_ptr<tx::transaction> m_tx;

        public:
            materialize_plan(std::shared_ptr<tx::transaction> tx, std::shared_ptr<i_plan> src_plan)
                : m_tx(tx), m_src_plan(src_plan)
            {}

            materialize_plan(std::shared_ptr<tx::transaction> tx, i_plan* src_plan)
                : m_tx(tx), m_src_plan(src_plan)
            {}

            std::shared_ptr<scan::i_scan> open() override
            {
                auto sch = m_src_plan->schema();
                temp_table temp_tbl(m_tx, sch);
                auto src = m_src_plan->open();
                auto dest = temp_tbl.open();

                while (src->next())
                {
                    dest->insert();
                    for (const auto& fldname : sch->fields())
                        dest->set_val(fldname, src->get_val(fldname));
                }

                src->close();
                dest->before_first();
                return dest;
            }

            std::size_t blocks_accessed() const override
            {
                auto sch = m_src_plan->schema();
                record::layout layout(std::make_shared<record::schema>(sch));
                double rpb = static_cast<double>(m_tx->block_size()) / layout.slot_size();
                return static_cast<std::size_t>(std::ceil(m_src_plan->records() / rpb));
            }

            std::size_t records() const override
            {
                return m_src_plan->records();
            }

            std::size_t distinct_values(const std::string& fldname) const override
            {
                return m_src_plan->distinct_values(fldname);
            }

            std::shared_ptr<record::schema> schema() override
            {
                return m_src_plan->schema();
            }
    };
}

#endif