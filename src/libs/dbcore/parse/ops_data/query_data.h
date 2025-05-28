#ifndef __PARSE_QUERY_DATA_H
#define __PARSE_QUERY_DATA_H

#include <vector>
#include <string>

#include "libs/dbcore/query/predicate.h"

namespace dbcore::parse
{
    class query_data
    {
        private:
            std::vector<std::string> m_fields;
            std::vector<std::string> m_tables;
            query::predicate m_pred;

        public:
            query_data(const std::vector<std::string>& fields,
                       const std::vector<std::string>& tables,
                       const query::predicate& pred)
                : m_fields(fields), m_tables(tables), m_pred(pred)
            {}

            std::vector<std::string> fields() const
            {
                return m_fields;
            }

            std::vector<std::string> tables() const
            {
                return m_tables;
            }

            query::predicate predicate() const
            {
                return m_pred;
            }

            std::string to_string() const
            {
                std::string result = "select ";
                for (const auto& fldname : m_fields)
                    result += fldname + ", ";

                if (!m_fields.empty())
                    result = result.substr(0, result.length() - 2);

                result += " from ";
                for (const auto& tblname : m_tables)
                    result += tblname + ", ";

                if (!m_tables.empty())
                    result = result.substr(0, result.length() - 2);

                std::string pred = m_pred.to_string();
                if (!pred.empty())
                    result += " where " + pred;

                return result;
            }
    };
}

#endif