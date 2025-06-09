#ifndef __MATERIALIZE_SORT_SCAN_H
#define __MATERIALIZE_SORT_SCAN_H

#include "libs/dbcore/scan/i_update_scan.h"
#include "libs/dbcore/record/record_id.h"
#include "record_comparator.h"
#include "temp_table.h"

using namespace dbcore::scan;

namespace dbcore::materialize
{
    class sort_scan : public i_scan
    {
        private:
            std::shared_ptr<i_update_scan> m_lhs_scan;
            std::shared_ptr<i_update_scan> m_rhs_scan;
            std::shared_ptr<i_update_scan> m_curr_scan;
            std::shared_ptr<record_comparator> m_rec_comp;
            bool m_lhs_has_more;
            bool m_rhs_has_more;
            std::pair<record::record_id, record::record_id> m_saved_position;
        
        public:
            sort_scan(const std::vector<std::shared_ptr<temp_table>> runs, std::shared_ptr<record_comparator> rec_comp)
                : m_rec_comp(rec_comp)
            {
                m_lhs_scan = runs[0]->open();
                m_lhs_has_more = m_lhs_scan->next();
                if (runs.size() > 1)
                {
                    m_rhs_scan = runs[1]->open();
                    m_rhs_has_more = m_rhs_scan->next();
                }
            }

            void before_first() override
            {
                m_curr_scan = nullptr;
                m_lhs_scan->before_first();
                m_lhs_has_more = m_lhs_scan->next();
                if (m_rhs_scan)
                {
                    m_rhs_scan->before_first();
                    m_rhs_has_more = m_rhs_scan->next();
                }
            }

            bool next() override
            {
                if (m_curr_scan)
                {
                    if (m_curr_scan == m_lhs_scan)
                        m_lhs_has_more = m_lhs_scan->next();
                    else if (m_curr_scan == m_rhs_scan)
                        m_rhs_has_more = m_rhs_scan->next();
                }

                if (!m_lhs_has_more && !m_rhs_has_more)
                    return false;
                else if (m_lhs_has_more && m_rhs_has_more)
                    m_curr_scan = m_rec_comp->compare(m_lhs_scan, m_rhs_scan) ? m_rhs_scan : m_lhs_scan;
                else if (m_lhs_has_more)
                    m_curr_scan = m_lhs_scan;
                else if (m_rhs_has_more)
                    m_curr_scan = m_rhs_scan;
                
                return true;
            }

            int get_int(const std::string& fldname) override
            {
                return m_curr_scan->get_int(fldname);
            }

            std::string get_str(const std::string& fldname) override
            {
                return m_curr_scan->get_str(fldname);            
            }

            query::constant get_val(const std::string& fldname) override
            {
                return m_curr_scan->get_val(fldname);
            }

            bool has_field(const std::string& fldname) override
            {
                return m_curr_scan->has_field(fldname);
            }

            void close() override
            {
                m_lhs_scan->close();
                if (m_rhs_scan)
                    m_rhs_scan->close();
            }

            void save_position()
            {
                auto rid1 = m_lhs_scan->get_rid();
                auto rid2 = m_rhs_scan->get_rid();
                m_saved_position = {rid1, rid2};
            }

            void restore_position()
            {
                auto rid1 = m_saved_position.first;
                auto rid2 = m_saved_position.second;
                m_lhs_scan->move_to_rid(rid1);
                if (rid2.block_number() != -1)
                    m_rhs_scan->move_to_rid(rid2);
            }
    };
}

#endif