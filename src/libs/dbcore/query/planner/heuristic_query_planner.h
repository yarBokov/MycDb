#ifndef __QUERY_HEURISTIC_QUERY_PLANNER_H
#define __QUERY_HEURISTIC_QUERY_PLANNER_H

#include "i_query_planner.h"
#include "table_planner.h"
#include "libs/dbcore/query/plan/project_plan.h"
#include "libs/dbcore/metadata/metadata_mgr.h"

#include <vector>

using namespace dbcore::query::plan;

namespace dbcore::query::optimization
{
    class heuristic_query_planner : public i_query_planner
    {
        private:
            std::vector<std::unique_ptr<table_planner>> m_table_planners;
            metadata::metadata_mgr& m_mdm;

            void erase_planners(table_planner* planner)
            {
                m_table_planners.erase(
                    std::remove_if(m_table_planners.begin(), m_table_planners.end(),
                        [planner](const std::unique_ptr<table_planner>& tp){
                            return tp.get() == planner;
                        }), m_table_planners.end());

                delete planner;
            }
            
            std::unique_ptr<i_plan> get_select_plan()
            {
                table_planner* planner = nullptr;
                std::unique_ptr<i_plan> best_plan;

                for (auto& tp : m_table_planners)
                {
                    auto plan = tp->construct_select_plan();
                    if (!best_plan || plan->records() < best_plan->records())
                    {
                        planner = tp.get();
                        best_plan = std::move(plan);
                    }
                }

                erase_planners(planner);

                return best_plan;
            }

            std::unique_ptr<i_plan> get_join_plan(std::unique_ptr<i_plan> curr)
            {
                table_planner* planner = nullptr;
                std::unique_ptr<i_plan> best_plan;

                for (auto& tp : m_table_planners)
                {
                    auto plan = tp->construct_join_plan(curr.get()).value();
                    if (plan && (!best_plan || plan->records() < best_plan->records()))
                    {
                        planner = tp.get();
                        best_plan = std::move(plan);
                    }
                }

                if (best_plan)
                    erase_planners(planner);

                return best_plan;
            }

            std::unique_ptr<i_plan> get_product_plan(std::unique_ptr<i_plan> curr)
            {
                table_planner* planner = nullptr;
                std::unique_ptr<i_plan> best_plan;

                for (auto& tp : m_table_planners)
                {
                    auto plan = tp->construct_product_plan(curr.get());
                    if (!best_plan || plan->records() < best_plan->records())
                    {
                        planner = tp.get();
                        best_plan = std::move(plan);
                    }
                }

                erase_planners(planner);

                return best_plan;
            }

        public:
            heuristic_query_planner(metadata::metadata_mgr& mdm)
                : m_mdm(mdm)
            {}

            std::unique_ptr<i_plan> create_plan(parse::query_data& data, tx::transaction& tx) override
            {
                for (const auto& tblname : data.tables())
                {
                    m_table_planners.push_back(std::make_unique<table_planner>(tblname, data.predicate(), 
                        std::make_shared<tx::transaction>(tx), m_mdm));
                }

                auto curr_plan = get_select_plan();

                while (!m_table_planners.empty())
                {
                    auto p = get_join_plan(std::move(curr_plan));
                    if (p)
                        curr_plan = std::move(p);
                    else
                        curr_plan = get_product_plan(std::move(curr_plan));
                }

                return std::make_unique<project_plan>(std::move(curr_plan), data.fields());
            }
    };
}

#endif