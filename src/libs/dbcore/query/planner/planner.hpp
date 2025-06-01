#ifndef __QUERY_PLANNER_HPP
#define __QUERY_PLANNER_HPP

#include "heuristic_query_planner.h"
#include "basic_update_planner.h"

#include "libs/dbcore/parse/parser.hpp"

namespace dbcore::query
{
    class planner
    {
        private:
            std::unique_ptr<i_query_planner> m_qry_planner;
            std::unique_ptr<i_update_planner> m_upd_planner;
            std::unique_ptr<parse::parser> m_parser;

        public:
            planner(std::unique_ptr<i_query_planner> qry_planner, 
                std::unique_ptr<i_update_planner> upd_planner, std::unique_ptr<parse::parser> parser)
                : m_qry_planner(std::move(qry_planner))
                , m_upd_planner(std::move(upd_planner))
                , m_parser(std::move(parser))
            {}

            std::unique_ptr<plan::i_plan> create_query_plan(std::string query, tx::transaction& tx)
            {
                m_parser->analyze(query);
                auto data = m_parser->query();
                return m_qry_planner->create_plan(*data, tx);
            }

            int execute_update(std::string command, tx::transaction& tx)
            {
                using namespace dbcore::parse;
                m_parser->analyze(command);
                auto cmd_result = m_parser->update_cmd();
                if (std::holds_alternative<insert_data>(cmd_result))
                {
                    auto data = std::get<insert_data>(cmd_result);
                    return m_upd_planner->execute_insert(data, tx);
                }
                else if (std::holds_alternative<delete_data>(cmd_result))
                {
                    auto data = std::get<delete_data>(cmd_result);
                    return m_upd_planner->execute_delete(data, tx);
                }
                else if (std::holds_alternative<modify_data>(cmd_result))
                {
                    auto data = std::get<modify_data>(cmd_result);
                    return m_upd_planner->execute_modify(data, tx);
                }
                else if (std::holds_alternative<create_table_data>(cmd_result))
                {
                    auto data = std::get<create_table_data>(cmd_result);
                    return m_upd_planner->execute_create_table(data, tx);
                }
                else if (std::holds_alternative<create_view_data>(cmd_result))
                {
                    auto data = std::get<create_view_data>(cmd_result);
                    return m_upd_planner->execute_create_view(data, tx);
                }
                else if (std::holds_alternative<create_index_data>(cmd_result))
                {
                    auto data = std::get<create_index_data>(cmd_result);
                    return m_upd_planner->execute_create_index(data, tx);
                }
                else return 0;
            }
    };
}

#endif