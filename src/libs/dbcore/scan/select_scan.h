#ifndef __SCAN_SELECTSCAN_H
#define __SCAN_SELECTSCAN_H

#include "i_update_scan.h"
#include "libs/dbcore/query/predicate.h"

#include <string>

namespace dbcore::scan
{
    class select_scan : public i_update_scan
    {
        private:
            std::unique_ptr<i_update_scan> m_update_scan;
            query::predicate m_pred;

        public:
            select_scan(std::unique_ptr<i_update_scan> update_scan, const query::predicate& pred)
                : m_update_scan(std::move(update_scan)), m_pred(pred)
            {}
            //scan

            void before_first() override
            {
                m_update_scan->before_first();
            }

            bool next() override
            {
                while (m_update_scan->next())
                {
                    if (m_pred.is_satisfied(*m_update_scan))
                        return true;
                }
                return false;
            }

            int get_int(const std::string& fldname) override
            {
                return m_update_scan->get_int(fldname);   
            }

            std::string get_str(const std::string& fldname) override
            {
                return m_update_scan->get_str(fldname);
            }

            query::constant get_val(const std::string& fldname) override
            {
                return m_update_scan->get_val(fldname);
            }

            bool has_field(const std::string& fldname) override
            {
                return m_update_scan->has_field(fldname);
            }

            void close() override
            {
                m_update_scan->close();
            }
            //update scan

            void set_val(const std::string& fldname, const query::constant& val) override
            {
                m_update_scan->set_val(fldname, val);
            }

            void set_int(const std::string& fldname, int val) override
            {
                m_update_scan->set_int(fldname, val);
            }

            void set_str(const std::string& fldname, const std::string& val) override
            {
                m_update_scan->set_str(fldname, val);
            }

            void insert() override
            {
                m_update_scan->insert();
            }

            void delete_record() override
            {
                m_update_scan->delete_record();
            }

            record::record_id get_rid() override
            {
                return m_update_scan->get_rid();
            }

            void move_to_rid(const record::record_id& rid) override
            {
                m_update_scan->move_to_rid(rid);
            }
    };
}

#endif