#ifndef __PARSE_PRED_PARSER_H
#define __PARSE_PRED_PARSER_H

#include "lexer.hpp"

#include <string>

namespace dbcore::parse
{
    class predicate_parser
    {
        private:
            lexer m_lexer;

        public:
            predicate_parser()
                : m_lexer()
            {}

            void analyze(const std::string& str)
            {
                m_lexer.analyze(str);
            }

            std::string get_field()
            {
                return m_lexer.consume_id();
            }

            void constant()
            {
                if (m_lexer.match_str_constant())
                    m_lexer.consume_str_constant();
                else
                    m_lexer.consume_int_constant();
            }

            void expression()
            {
                if (m_lexer.match_id())
                    get_field();
                else
                    constant();
            }

            void term()
            {
                expression();
                m_lexer.consume_delimeter('=');
                expression();
            }

            void predicate()
            {
                term();
                if (m_lexer.match_keyword("and"))
                {
                    m_lexer.consume_keyword("and");
                    predicate();
                }
            }
    };
}

#endif