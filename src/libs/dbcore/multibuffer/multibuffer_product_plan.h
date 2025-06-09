#ifndef __MULTIBUFFER_PRODUCT_PLAN_H
#define __MULTIBUFFER_PRODUCT_PLAN_H

#include "libs/dbcore/query/plan/i_plan.h"
#include "libs/dbcore/tx/transaction.h"
#include "libs/dbcore/record/schema.h"
#include "multibuffer_product_scan.h"

#include "libs/dbcore/materialize/temp_table.h"
#include "libs/dbcore/materialize/materialize_plan.h"

using namespace dbcore::query::plan;

namespace dbcore::multibuffer
{
    class multibuffer_product_plan : public i_plan
    {
        private:
            std::shared_ptr<tx::transaction> m_tx;
            std::unique_ptr<i_plan> m_lhs;
            i_plan* m_rhs;
            std::shared_ptr<record::schema> m_sch;
            
            materialize::temp_table copy_records_from(i_plan* p)
            {
                auto src = p->open();
                auto sch = p->schema();
                materialize::temp_table t(m_tx, m_sch);
                auto dest = std::dynamic_pointer_cast<scan::i_update_scan>(t.open());
                while (src->next())
                {
                    dest->insert();
                    for (const auto& fldname : m_sch->fields())
                        dest->set_val(fldname, src->get_val(fldname));
                }
                src->close();
                dest->close();
                return t;
            }

        public:
            multibuffer_product_plan(std::shared_ptr<tx::transaction> tx, i_plan* lhs, i_plan* rhs)
                : m_tx(tx), m_lhs(std::make_unique<materialize::materialize_plan>(lhs)), m_rhs(rhs)
            {
                m_sch->add_all(m_lhs->schema());
                m_sch->add_all(m_rhs->schema());
            }

            ~multibuffer_product_plan()
            {
                delete m_rhs;
            }

            std::shared_ptr<scan::i_scan> open() override
            {
                auto left_scan = m_lhs->open();
                materialize::temp_table tt = copy_records_from(m_rhs);
                return std::make_shared<multibuffer_product_scan>(m_tx, left_scan, tt.get_table_name(), tt.get_layout());
            }
            
            std::size_t blocks_accessed() const override
            {
                auto available = m_tx->available_buffers_count();
                materialize::materialize_plan m_p(m_tx, m_rhs);
                auto size = m_p.blocks_accessed();
                int num_chunks = size / available;
                return m_rhs->blocks_accessed() + (m_lhs->blocks_accessed() * num_chunks);
            }

            std::size_t records() const override
            {
                return m_lhs->records() * m_rhs->records();
            }

            std::size_t distinct_values(const std::string& fldname) const override
            {
                if (m_lhs->schema()->has_field(fldname))
                    m_lhs->distinct_values(fldname);
                else 
                    m_rhs->distinct_values(fldname);
            }

            std::shared_ptr<record::schema> schema() override
            {
                return m_sch;
            }
    };
}

#endif