#ifndef __MATERIALIZE_MERGE_JOIN_SCAN_H
#define __MATERIALIZE_MERGE_JOIN_SCAN_H

#include "sort_scan.h"
#include "libs/dbcore/query/constant.h"

namespace dbcore::materialize
{
    class merge_join_scan : public i_scan
    {
        private:
            std::shared_ptr<i_scan> m_lhs_scan;
            std::shared_ptr<sort_scan> m_rhs_scan;
            std::string m_fldname1;
            std::string m_fldname2;
            query::constant m_join_val;

        public:
            merge_join_scan(std::shared_ptr<i_scan> lhs_scan, std::shared_ptr<sort_scan> rhs_scan,
                const std::string& fldname1, const std::string& fldname2)
                : m_lhs_scan(lhs_scan), m_rhs_scan(rhs_scan), m_fldname1(fldname1), m_fldname2(fldname2)
            {
                before_first();
            }
  
            void before_first() override
            {
                m_lhs_scan->before_first();
                m_rhs_scan->before_first();
            }

            bool next() override
            {
                bool has_more2 = m_rhs_scan->next();
                if (has_more2 && m_rhs_scan->get_val(m_fldname2) == m_join_val)
                    return true;

                bool has_more1 = m_lhs_scan->next();
                if (has_more1 && m_lhs_scan->get_val(m_fldname1) == m_join_val)
                {
                    m_rhs_scan->restore_position();
                    return true;
                }
                
                while (has_more1 && has_more2)
                {
                    auto val1 = m_lhs_scan->get_val(m_fldname1);
                    auto val2 = m_rhs_scan->get_val(m_fldname2);

                    if (val1.compare_to(val2) < 0)
                        has_more1 = m_lhs_scan->next();
                    else if (val1.compare_to(val2) > 0)
                        has_more2 = m_rhs_scan->next();
                    else
                    {
                        m_rhs_scan->save_position();
                        m_join_val = m_rhs_scan->get_val(m_fldname2);
                        return true;
                    }
                } 
                return false;
            }

            int get_int(const std::string& fldname) override
            {
                if (m_lhs_scan->has_field(fldname))
                    return m_lhs_scan->get_int(fldname);
                
                return m_rhs_scan->get_int(fldname);
            }

            std::string get_str(const std::string& fldname) override
            {
                if (m_lhs_scan->has_field(fldname))
                    return m_lhs_scan->get_str(fldname);
                
                return m_rhs_scan->get_str(fldname);
            }

            query::constant get_val(const std::string& fldname) override
            {
                if (m_lhs_scan->has_field(fldname))
                    return m_lhs_scan->get_val(fldname);
                
                return m_rhs_scan->get_val(fldname);
            }

            bool has_field(const std::string& fldname) override
            {
                return m_lhs_scan->has_field(fldname) || m_rhs_scan->has_field(fldname);
            }

            void close() override
            {
                m_lhs_scan->close();
                m_rhs_scan->close();
            }
    };
}

#endif