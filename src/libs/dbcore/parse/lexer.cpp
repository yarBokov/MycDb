#include "lexer.hpp"

#include <algorithm>

#include "libs/common/config.hpp"

#include "libs/dbcore/exceptions/bad_syntax_exception.h"

namespace dbcore::parse
{
    lexer::lexer()
        : m_pos(0)
    {
        init_keywords();
    }

    void lexer::analyze(const std::string& str)
    {
        m_input = str;
        next_token();
    }

    void lexer::init_keywords()
    {
        std::ifstream keyword_file(config::sql_query_dict);
        std::string keyword;
        while (std::getline(keyword_file, keyword))
            m_keywords.push_back(keyword);
    }

    bool lexer::match_delimeter(char delim)
    {
        return m_curr_type == delim;
    }

    void lexer::consume_delimeter(char delim)
    {
        if (!match_delimeter(delim))
            throw_error("Expected a delimeter: " + delim);

        next_token();
    }

    bool lexer::match_int_constant()
    {
        return m_curr_type == '0';
    }

    int lexer::consume_int_constant()
    {
        if (!match_int_constant())
            throw_error("Expected integer constant");

        int i = static_cast<int>(m_curr_number);
        next_token();
        return i;
    }

    bool lexer::match_str_constant()
    {
        return m_curr_type == '\'';
    }

    std::string lexer::consume_str_constant()
    {
        if (!match_str_constant())
            throw_error("Expected string constant");
        
        std::string str = m_curr_string;
        next_token();
        return str;
    }

    bool lexer::match_keyword(const std::string& word)
    {
        return m_curr_type == 'a' && m_curr_string == word;
    }

    void lexer::consume_keyword(const std::string& word)
    {
        if (!match_keyword(word))
            throw_error("Expected keyword " + word);
        next_token();
    }

    bool lexer::match_id()
    {
        return m_curr_type == 'a' && 
            std::find(m_keywords.begin(), m_keywords.end(), m_curr_string) == m_keywords.end();
    }

    std::string lexer::consume_id()
    {
        if (!match_id())
            throw_error("Expected identifier");
        
        std::string str = m_curr_string;
        next_token();
        return str;
    }

    void lexer::next_token()
    {
        skip_whitespace();

        if (m_pos >= m_input.length())
        {
            m_curr_type = EOF;
            return;
        }

        m_curr_char = m_input[m_pos];

        if (std::isdigit(m_curr_char))
            read_number();
        else if (std::isalpha(m_curr_char) || m_curr_char == '_')
            read_word();
        else if (m_curr_char == '\'')
            read_string();
        else
        {
            m_curr_type = m_curr_char;
            m_pos++;
        }
    }

    void lexer::read_string()
    {
        m_pos++;
        std::ostringstream oss;
        while (m_pos < m_input.length() && m_input[m_pos] != '\'')
            oss << m_input[m_pos++];

        if (m_pos >= m_input.length())
            throw_error("Unterminated string");
        m_pos++;
        m_curr_string = oss.str();
        m_curr_type = '\'';
    }

    void lexer::read_number()
    {
        std::ostringstream oss;
        while (m_pos < m_input.length() && std::isdigit(m_input[m_pos]))
            oss << m_input[m_pos++];

        m_curr_number = std::stod(oss.str());
        m_curr_type = '0';
    }

    void lexer::read_word()
    {
        std::ostringstream oss;
        while (m_pos < m_input.length() && (std::isalnum(m_input[m_pos]) || m_input[m_pos] == '_'))
            oss << static_cast<char>(std::tolower(m_input[m_pos++]));

        m_curr_string = oss.str();
        m_curr_type = 'a';
    }

    void lexer::skip_whitespace()
    {
        while (m_pos < m_input.length() && std::isspace(m_input[m_pos]))
            m_pos++;
    }

    void lexer::throw_error(const std::string& msg)
    {
        throw exceptions::bad_syntax_exception();
    }
}