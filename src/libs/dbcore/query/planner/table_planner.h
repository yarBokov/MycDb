#ifndef __QUERY_TABLE_PLANNER_H
#define __QUERY_TABLE_PLANNER_H

#include "libs/dbcore/query/plan/table_plan.h"
#include "libs/dbcore/query/plan/select_plan.h"
#include "libs/dbcore/record/schema.h"
#include "libs/dbcore/query/predicate.h"
#include "libs/dbcore/metadata/index_info.h"
#include "libs/dbcore/metadata/metadata_mgr.h"

#include <unordered_map>
#include <optional>

namespace dbcore::query::optimization
{
    class table_planner
    {
        private:
            std::unique_ptr<query::plan::table_plan> m_tbl_plan;
            predicate m_pred;
            record::schema m_sch;
            std::unordered_map<std::string, metadata::index_info> m_indexes;
            std::shared_ptr<tx::transaction> m_tx;

            std::unique_ptr<query::plan::i_plan> construct_index_select()
            {
                for (const auto& [fldname, ii] : m_indexes)
                {
                    auto val = m_pred.equates_with_constant(fldname);
                    if (val.as_int() != 0 && val.as_str() != "")
                    {
                        // std::cout << usd index on << fldname;
                        return std::make_unique<index_select_plan>(
                            std::unique_ptr<query::plan::i_plan>(), ii, val);
                        );
                    }
                }
                return nullptr;
            }

            std::optional<std::unique_ptr<query::plan::i_plan>> construct_index_join(query::plan::i_plan* curr, const record::schema& currsch)
            {
                for (const auto& [fldname, ii] : m_indexes)
                {
                    auto outer_field = m_pred.equates_with_field(fldname);
                    if (currsch.has_field(outer_field))
                    {
                        auto p = std::make_unique<index_join_plan>(
                            curr, std::unique_ptr<query::plan::i_plan>(m_tbl_plan.release()), ii, outer_field);
                        p = add_select_predicate(std::move(p));
                        return add_join_predicate(std::move(p), currsch);
                    }
                }
                return std::nullopt;
            }

            std::unique_ptr<query::plan::i_plan> construct_product_join(query::plan::i_plan* curr, const record::schema& currsch)
            {
                auto p = construct_product_plan(curr);
                return add_join_predicate(std::move(p), currsch);
            }
    
            std::unique_ptr<query::plan::i_plan> add_select_predicate(std::unique_ptr<query::plan::i_plan> p)
            {
                auto selectpred = m_pred.select_sub_pred(m_sch);
                if (selectpred)
                    return std::make_unique<query::plan::select_plan>(std::move(p), *selectpred);
                return p;
            }

            std::unique_ptr<query::plan::i_plan> add_join_predicate(std::unique_ptr<query::plan::i_plan> p, const record::schema& currsch)
            {
                auto joinpred = m_pred.join_sub_pred(currsch, m_sch);
                if (joinpred)
                    return std::make_unique<query::plan::select_plan>(std::move(p), *joinpred);
                return p;
            }

        public:
            table_planner(const std::string& tblname, predicate& pred,
                         std::shared_ptr<tx::transaction> tx, metadata::metadata_mgr& mdm)
                : m_pred(pred)
                , m_tx(tx)
            {
                m_tbl_plan = std::make_unique<query::plan::table_plan>();
                m_sch = m_tbl_plan->schema();
                m_indexes = mdm.get_index_info(tblname, *m_tx);
            }

            std::unique_ptr<query::plan::i_plan> construct_select_plan()
            {
                auto p = construct_index_select();
                if (!p)
                    p = std::unique_ptr<query::plan::i_plan>(m_tbl_plan.release());
                return add_select_predicate(std::move(p));
            }

            std::optional<std::unique_ptr<query::plan::i_plan>> construct_join_plan(query::plan::i_plan* curr)
            {
                auto currsch = curr->schema();
                auto joinpred = m_pred.join_sub_pred(m_sch, currsch);
                if (!joinpred)
                    return std::nullopt;

                auto p = construct_index_join(curr, currsch);
                if (!p)
                    p = std::make_unique<query::plan::i_plan>(construct_product_join(curr, currsch));

                return p;
            }

            std::unique_ptr<query::plan::i_plan> construct_product_plan(query::plan::i_plan* curr)
            {
                auto p = add_select_predicate(std::make_unique<query::plan::i_plan>(m_tbl_plan.release()));
                return std::make_unique<multibuffer_product_plan>(*m_tx, curr, p.release());
            }
    };
}

#endif