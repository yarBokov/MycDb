#ifndef __PARSE_INSERT_DATA_H
#define __PARSE_INSERT_DATA_H

#include <string>
#include <vector>

#include "libs/dbcore/query/constant.h"

namespace dbcore::parse
{
    class insert_data
    {
        private:
            std::string m_tblname;
            std::vector<std::string> m_fields;
            std::vector<query::constant> m_vals;

        public:
            insert_data(const std::string& tblname,
                        const std::vector<std::string>& fields,
                        const std::vector<query::constant>& vals)
                : m_tblname(tblname), m_fields(fields), m_vals(vals)
            {}

            std::string table_name() const
            {
                return m_tblname;
            }

            std::vector<std::string> fields() const
            {
                return m_fields;
            }

            std::vector<query::constant> vals() const
            {
                return m_vals;
            }
    };
}

#endif