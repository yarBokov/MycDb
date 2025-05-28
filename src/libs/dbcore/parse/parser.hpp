#ifndef __PARSE_LEXER_H
#define __PARSE_LEXER_H

#include "ops_data/create_index_data.h"
#include "ops_data/create_table_data.h"
#include "ops_data/create_view_data.h"
#include "ops_data/delete_data.h"
#include "ops_data/modify_data.h"
#include "ops_data/query_data.h"
#include "ops_data/insert_data.h"
#include "lexer.hpp"

#include <memory>

namespace dbcore::parse
{
    class parser
    {
        private:
            lexer m_lexer;

            std::vector<std::string> select_list();
            std::vector<std::string> table_list();
            std::vector<std::string> field_list();
            std::vector<std::shared_ptr < query::constant > > const_list();

            std::shared_ptr<void> create();

            record::schema field_defs();
            record::schema field_def();
            record::schema field_type(const std::string& fldname);

        public:
            parser();

            void analyze(const std::string& str);

            std::string get_field();
            std::shared_ptr<query::constant> constant();
            std::shared_ptr<query::expression> expression();
            std::shared_ptr<query::term> term();
            std::shared_ptr<query::predicate> predicate();
            
            std::shared_ptr<query_data> query();
            
            std::shared_ptr<void> update_cmd();  
            std::shared_ptr<delete_data> delete_cmd();
            std::shared_ptr<insert_data> insert();
            std::shared_ptr<modify_data> modify();
            std::shared_ptr<create_table_data> create_table();
            std::shared_ptr<create_view_data> create_view();
            std::shared_ptr<create_index_data> create_index();
    };
}

#endif