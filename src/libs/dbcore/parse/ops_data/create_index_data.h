#ifndef __PARSE_CREATE_INDEX_DATA_H
#define __PARSE_CREATE_INDEX_DATA_H

#include <string>

namespace dbcore::parse
{
    class create_index_data
    {
        private:
            std::string m_tblname;
            std::string m_fldname;
            std::string m_idxname;

        public:
            create_index_data(const std::string& tblname, 
                              const std::string& fldname,
                              const std::string& idxname)
                : m_tblname(tblname), m_fldname(fldname), m_idxname(idxname)
            {}

            std::string table_name() const
            {
                return m_tblname;
            }

            std::string field_name() const
            {
                return m_fldname;
            }

            std::string index_name() const
            {
                return m_idxname;
            }
    };
}

#endif