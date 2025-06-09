#ifndef __PARSE_CREATE_TABLE_DATA_H
#define __PARSE_CREATE_TABLE_DATA_H

#include <string>

#include "libs/dbcore/record/schema.h"

namespace dbcore::parse
{
    class create_table_data
    {
        private:
            std::string m_tblname;
            std::shared_ptr<record::schema> m_sch;

        public:
            create_table_data(const std::string& tblname, std::shared_ptr<record::schema> sch)
                : m_tblname(tblname), m_sch(sch)
            {}

            std::string table_name() const
            {
                return m_tblname;
            }

            std::shared_ptr<record::schema> new_schema() const
            {
                return m_sch;
            }
    };
}

#endif