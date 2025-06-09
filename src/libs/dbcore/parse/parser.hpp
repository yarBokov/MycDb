#ifndef __PARSE_PARSER_H
#define __PARSE_PARSER_H

#include "ops_data/create_index_data.h"
#include "ops_data/create_table_data.h"
#include "ops_data/create_view_data.h"
#include "ops_data/delete_data.h"
#include "ops_data/modify_data.h"
#include "ops_data/query_data.h"
#include "ops_data/insert_data.h"
#include "lexer.hpp"

#include <memory>
#include <variant>

namespace dbcore::parse
{
    class parser
    {
        private:
            lexer m_lexer;

            std::vector<std::string> select_list();
            std::vector<std::string> table_list();
            std::vector<std::string> field_list();
            std::vector<query::constant> const_list();

            std::shared_ptr<record::schema> field_defs();
            std::shared_ptr<record::schema> field_def();
            std::shared_ptr<record::schema> field_type(const std::string& fldname);

        public:
            using update_cmd_parse_result = std::variant<insert_data, delete_data, modify_data, 
                create_table_data, create_view_data, create_index_data>;

            parser();
            void analyze(const std::string& str);

            std::string get_field();
            query::constant constant();
            std::shared_ptr<query::expression> expression();
            std::shared_ptr<query::term> term();
            std::shared_ptr<query::predicate> predicate();
            
            std::shared_ptr<query_data> query();
            
            update_cmd_parse_result update_cmd();
            delete_data delete_cmd();
            insert_data insert();
            modify_data modify();
            create_table_data create_table();
            create_view_data create_view();
            create_index_data create_index();
    };
}

#endif