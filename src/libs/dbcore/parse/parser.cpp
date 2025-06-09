#include "parser.hpp"

#include <variant>

namespace dbcore::parse
{
    parser::parser()
        : m_lexer()
    {}

    void parser::analyze(const std::string& str)
    {
        m_lexer.analyze(str);
    }

    std::vector<std::string> parser::select_list()
    {
        std::vector < std::string > l;
        l.push_back(get_field());
        if (m_lexer.match_delimeter(','))
        {
            m_lexer.consume_delimeter(',');
            auto rest = select_list();
            l.insert(l.end(), rest.begin(), rest.end());
        }
        return l;
    }

    std::vector<std::string> parser::table_list()
    {
        std::vector < std::string > l;
        l.push_back(m_lexer.consume_id());
        if (m_lexer.match_delimeter(','))
        {
            m_lexer.consume_delimeter(',');
            auto rest = table_list();
            l.insert(l.end(), rest.begin(), rest.end());
        }
        return l;
    }

    std::vector<std::string> parser::field_list()
    {
        std::vector < std::string > l;
        l.push_back(get_field());
        if (m_lexer.match_delimeter(','))
        {
            m_lexer.consume_delimeter(',');
            auto rest = field_list();
            l.insert(l.end(), rest.begin(), rest.end());
        }
        return l;
    }

    std::vector<query::constant> parser::const_list()
    {
        std::vector < query::constant > l;
        l.push_back(constant()); 
        if (m_lexer.match_delimeter(','))
        {
            m_lexer.consume_delimeter(',');
            auto rest = const_list();
            l.insert(l.end(), rest.begin(), rest.end());
        }
        return l;
    }

    std::shared_ptr<record::schema> parser::field_defs()
    {
        auto sch = field_def();
        if (m_lexer.match_delimeter(','))
        {
            m_lexer.consume_delimeter(',');
            auto sch_2 = field_defs();
            sch->add_all(sch_2);
        }
        return sch;
    }

    std::shared_ptr<record::schema> parser::field_def()
    {
        std::string fldname = get_field();
        return field_type(fldname);
    }

    std::shared_ptr<record::schema> parser::field_type(const std::string& fldname)
    {
        std::shared_ptr<record::schema> sch;
        if (m_lexer.match_keyword("int"))
        {
            m_lexer.consume_keyword("int");
            sch->add_int_field(fldname);
        }
        else
        {
            m_lexer.consume_keyword("varchar");
            m_lexer.consume_delimeter('(');
            int strlen = m_lexer.consume_int_constant();
            m_lexer.consume_delimeter(')');
            sch->add_str_field(fldname, strlen);
        }
        return sch;
    }

    std::string parser::get_field()
    {
        return m_lexer.consume_id();
    }

    query::constant parser::constant()
    {
        if (m_lexer.match_str_constant())
            return query::constant(m_lexer.consume_str_constant());
        else
            return query::constant(m_lexer.consume_int_constant());
    }

    std::shared_ptr<query::expression> parser::expression()
    {
        if (m_lexer.match_id())
            return std::make_shared<query::expression>(get_field());
        else
            return std::make_shared<query::expression>(constant());
    }

    std::shared_ptr<query::term> parser::term()
    {
        auto lhs = expression();
        m_lexer.consume_delimeter('=');
        auto rhs = expression();
        return std::make_shared<query::term>(*lhs, *rhs);
    }

    std::shared_ptr<query::predicate> parser::predicate()
    {
        auto pred = std::make_shared<query::predicate>(term());
        if (m_lexer.match_keyword("and"))
        {
            m_lexer.consume_keyword("and");
            pred->conjoin_with(*predicate());
        }
        return pred;
    }
    
    std::shared_ptr<query_data> parser::query()
    {
        m_lexer.consume_keyword("select");
        auto fields = select_list();
        m_lexer.consume_keyword("from");
        auto tables = table_list();
        auto pred = std::make_shared<query::predicate>();
        if (m_lexer.match_keyword("where"))
        {
            m_lexer.consume_keyword("where");
            pred = predicate();
        }
        return std::make_shared<query_data>(fields, tables, *pred);
    }
    
    parser::update_cmd_parse_result parser::update_cmd()
    {
        if (m_lexer.match_keyword("insert"))
            return insert();
        else if (m_lexer.match_keyword("delete"))
            return delete_cmd();
        else if (m_lexer.match_keyword("update"))
            return modify();
        else
        {
            m_lexer.consume_keyword("create");
            if (m_lexer.match_keyword("table"))
                return create_table();
            else if (m_lexer.match_keyword("view"))
                return create_view();
            else 
                return create_index();
        }
    }

    delete_data parser::delete_cmd()
    {
        m_lexer.consume_keyword("delete");
        m_lexer.consume_keyword("from");
        std::string tblname = m_lexer.consume_id();
        auto pred = std::make_shared<query::predicate>();
        if (m_lexer.match_keyword("where"))
        {
            m_lexer.consume_keyword("where");
            pred = predicate();
        }
        return delete_data(tblname, *pred);
    }

    insert_data parser::insert()
    {
        m_lexer.consume_keyword("insert");
        m_lexer.consume_keyword("into");
        std::string tblname = m_lexer.consume_id();
        m_lexer.consume_delimeter('(');
        auto fields = field_list();
        m_lexer.consume_delimeter(')');
        m_lexer.consume_keyword("values");
        m_lexer.consume_delimeter('(');
        auto vals = const_list();
        m_lexer.consume_delimeter(')');
        return insert_data(tblname, fields, vals);
    }

    modify_data parser::modify()
    {
        m_lexer.consume_keyword("update");
        std::string tblname = m_lexer.consume_id();
        m_lexer.consume_keyword("set");
        std::string fldname = get_field();
        m_lexer.consume_delimeter('=');
        auto newval = expression();
        auto pred = std::make_shared<query::predicate>();
        if (m_lexer.match_keyword("where"))
        {
            m_lexer.consume_keyword("where");
            pred = predicate();
        }
        return modify_data(tblname, fldname, *newval, *pred);
    }

    create_table_data parser::create_table()
    {
        m_lexer.consume_keyword("table");
        std::string tblname = m_lexer.consume_id();
        m_lexer.consume_delimeter('(');
        auto sch = field_defs();
        m_lexer.consume_delimeter(')');
        return create_table_data(tblname, sch);
    }

    create_view_data parser::create_view()
    {
        m_lexer.consume_keyword("view");
        std::string viewname = m_lexer.consume_id();
        m_lexer.consume_keyword("as");
        auto qd = query();
        return create_view_data(viewname, *qd);
    }

    create_index_data parser::create_index()
    {
        m_lexer.consume_keyword("index");
        std::string idxname = m_lexer.consume_id();
        m_lexer.consume_keyword("on");
        std::string tblname = m_lexer.consume_id();
        m_lexer.consume_delimeter('(');
        std::string fldname = get_field();
        m_lexer.consume_delimeter(')');
        return create_index_data(idxname, tblname, fldname);
    }
}