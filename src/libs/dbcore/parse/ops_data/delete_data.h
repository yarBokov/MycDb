#ifndef __PARSE_DELETE_DATA_H
#define __PARSE_DELETE_DATA_H

#include <string>
#include "libs/dbcore/query/predicate.h"

namespace dbcore::parse
{
    class delete_data
    {
        private:
            std::string m_tblname;
            query::predicate m_pred;

        public:
            delete_data(const std::string& tblname, const query::predicate& pred)
                : m_tblname(tblname), m_pred(pred)
            {}

            std::string table_name() const
            {
                return m_tblname;
            }

            query::predicate pred() const
            {
                return m_pred;
            }
    };
}

#endif