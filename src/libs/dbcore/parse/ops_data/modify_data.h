#ifndef __PARSE_MODIFY_DATA_H
#define __PARSE_MODIFY_DATA_H

#include <string>

#include "libs/dbcore/query/expression.h"
#include "libs/dbcore/query/predicate.h"

namespace dbcore::parse
{
    class modify_data
    {
        private:
            std::string m_tblname;
            std::string m_fldname;
            query::expression m_newval;
            query::predicate m_pred;

        public:
            modify_data(const std::string& tblname,
                       const std::string& fldname,
                       const query::expression& newval,
                       const query::predicate& pred)
                : m_tblname(tblname), m_fldname(fldname), m_newval(newval), m_pred(pred)
            {}

            std::string table_name() const
            {
                return m_tblname;
            }

            std::string target_field() const
            {
                return m_fldname;
            }

            query::expression new_value() const
            {
                return m_newval;
            }

            query::predicate pred() const
            {
                return m_pred;
            }
    };
}

#endif