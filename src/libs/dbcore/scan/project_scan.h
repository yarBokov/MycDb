#ifndef __SCAN_PROJECTSCAN_H
#define __SCAN_PROJECTSCAN_H

#include "i_scan.h"
#include "libs/dbcore/query/constant.h"

#include <vector>
#include <string>
#include <memory>
#include <algorithm>

namespace dbcore::scan
{
    class project_scan : public i_scan
    {
        private:
            std::unique_ptr<i_scan> m_scan;
            std::vector<std::string> m_field_list;

        public:
            project_scan(std::unique_ptr<i_scan> scan, const std::vector<std::string>& field_list)
                : m_scan(std::move(scan)), m_field_list(field_list)
            {}

            //scan
            void before_first() override
            {
                m_scan->before_first();
            }

            bool next() override
            {
                return m_scan->next();
            }

            bool has_field(const std::string& fldname) override
            {
                return std::find(m_field_list.begin(), m_field_list.end(), fldname) != m_field_list.end();
            }

            void close() override
            {
                m_scan->close();
            }

            int get_int(const std::string& fldname) override
            {
                if (has_field(fldname))
                    return m_scan->get_int(fldname);
                throw std::runtime_error("Field " + fldname + " not found, method: get_int, project_scan.h");
            }

            std::string get_str(const std::string& fldname) override
            {
                if (has_field(fldname))
                    return m_scan->get_str(fldname);
                throw std::runtime_error("Field " + fldname + " not found, method: get_str, project_scan.h");
            }

            query::constant get_val(const std::string& fldname) override
            {
                if (has_field(fldname))
                    return m_scan->get_val(fldname);
                throw std::runtime_error("Value of field " + fldname + " not found, method: get_val, project_scan.h");
            }
    };
}

#endif