#ifndef __PARSE_LEXER_HPP
#define __PARSE_LEXER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>

namespace dbcore::parse
{
    class lexer
    {
        private:
            std::vector<std::string> m_keywords;
            std::string m_input;
            std::size_t m_pos;
            char m_curr_char;
            int m_curr_type;
            std::string m_curr_string;
            double m_curr_number;

            void next_token();
            void skip_whitespace();
            void read_string();
            void read_number();
            void read_word();
            void init_keywords();
            void throw_error(const std::string& msg);

        public:
            lexer();

            void analyze(const std::string& str);

            [[nodiscard]] bool match_delimeter(char delim);
            [[nodiscard]] bool match_int_constant();
            [[nodiscard]] bool match_str_constant();
            [[nodiscard]] bool match_keyword(const std::string& word);
            [[nodiscard]] bool match_id();

            void consume_delimeter(char delim);
            int consume_int_constant();
            std::string consume_str_constant();
            void consume_keyword(const std::string& str);
            std::string consume_id();
    };
}

#endif