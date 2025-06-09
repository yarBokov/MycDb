#ifndef __MATERIALIZE_GROUP_BY_SCAN_H
#define __MATERIALIZE_GROUP_BY_SCAN_H

#include "group_value.h"
#include "fn/i_aggregate_fn.h"

using namespace dbcore::scan;

namespace dbcore::materialize
{
    class group_by_scan : public i_scan
    {
        private:
            std::shared_ptr<i_scan> m_scan;
            std::vector<std::string> m_group_fields;
            std::vector<std::shared_ptr<i_aggregate_fn>> m_aggr_funcs;
            std::shared_ptr<group_value> m_group_val;
            bool more_groups;

        public:
            group_by_scan(std::shared_ptr<i_scan> scan, const std::vector<std::string>& group_fields,
                 const std::vector<std::shared_ptr<i_aggregate_fn>>& aggr_funcs)
                : m_scan(scan), m_group_fields(group_fields), m_aggr_funcs(aggr_funcs)
            {
                before_first();
            }

            void before_first() override
            {
                m_scan->before_first();
                more_groups = m_scan->next();
            }

            bool next() override
            {
                if (!more_groups)
                    return false;

                for (auto& fn : m_aggr_funcs)
                    fn->process_first(m_scan);

                m_group_val = std::make_shared<group_value>(m_scan, m_group_fields);

                while (more_groups = m_scan->next())
                {
                    auto gv = std::make_shared<group_value>(m_scan, m_group_fields);
                    if (!m_group_val->equals(gv))
                        break;

                    for (auto& fn : m_aggr_funcs)
                        fn->process_next(m_scan);
                }

                return true;
            }

            int get_int(const std::string& fldname) override
            {
                return get_val(fldname).as_int();
            }

            std::string get_str(const std::string& fldname) override
            {
                return get_val(fldname).as_str();
            }

            query::constant get_val(const std::string& fldname) override
            {
                if (std::find(m_group_fields.begin(), m_group_fields.end(), fldname) != m_group_fields.end())
                {
                    return m_group_val->get_val(fldname);
                }

                for (auto& fn : m_aggr_funcs)
                {
                    if (fn->field_name() == fldname)
                        return fn->value();
                }
                throw std::runtime_error("Field " + fldname + " was not found, file: group_by_scan.h");
            }

            bool has_field(const std::string& fldname) override
            {
                if (std::find(m_group_fields.begin(), m_group_fields.end(), fldname) != m_group_fields.end())
                {
                    return true;
                }

                for (auto& fn : m_aggr_funcs)
                {
                     if (fn->field_name() == fldname)
                        return true;
                }
                return false;
            }

            void close() override
            {
                m_scan->close();
            }
    };
}

#endif