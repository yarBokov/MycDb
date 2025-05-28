#ifndef __PARSE_CREATE_VIEW_DATA_H
#define __PARSE_CREATE_VIEW_DATA_H

#include <string>

#include "query_data.h"

namespace dbcore::parse
{
    class create_view_data
    {
        private:
            std::string m_viewname;
            query_data m_qrydata;

        public:
            create_view_data(const std::string& viewname, const query_data& qrydata)
                : m_viewname(viewname), m_qrydata(qrydata)
            {}

            std::string view_name() const
            {
                return m_viewname;
            }

            std::string view_def() const
            {
                return m_qrydata.to_string();
            }
    };
}

#endif