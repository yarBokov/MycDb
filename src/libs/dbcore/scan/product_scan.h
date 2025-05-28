#ifndef __SCAN_PRODUCTSCAN_H
#define __SCAN_PRODUCTSCAN_H

#include "i_scan.h"

#include <memory>

namespace dbcore::scan
{
    class product_scan : public i_scan
    {
        private:
            std::unique_ptr<i_scan> m_left_scan;
            std::unique_ptr<i_scan> m_right_scan;

        public:
            void before_first() override
            {
                m_left_scan->before_first();
                m_left_scan->next();
                m_right_scan->before_first();
            }

            bool next() override
            {
                if (m_right_scan->next())
                    return true;
                else
                {
                    m_right_scan->before_first();
                    return m_right_scan->next() && m_left_scan->next();
                }
            }

            product_scan(std::unique_ptr<i_scan> lhs_scan, std::unique_ptr<i_scan> rhs_scan)
                : m_left_scan(std::move(lhs_scan)), m_right_scan(std::move(rhs_scan))
            {
                before_first();
            }

            int get_int(const std::string& fldname) override
            {
                if (m_left_scan->has_field(fldname))
                    return m_left_scan->get_int(fldname);
                else
                    return m_right_scan->get_int(fldname);
            }

            std::string get_str(const std::string& fldname) override
            {
                if (m_left_scan->has_field(fldname))
                    return m_left_scan->get_str(fldname);
                else
                    return m_right_scan->get_str(fldname);
            }

            query::constant get_val(const std::string& fldname) override
            {
                if (m_left_scan->has_field(fldname))
                    return m_left_scan->get_val(fldname);
                else
                    return m_right_scan->get_val(fldname);
            }

            bool has_field(const std::string& fldname) override
            {
                return m_left_scan->has_field(fldname) || m_right_scan->has_field(fldname);
            }

            void close() override
            {
                m_left_scan->close();
                m_right_scan->close();
            }
    };
}

#endif