#ifndef __MATERIALIZE_AGGREGATE_COMPARE_FN_H
#define __MATERIALIZE_AGGREGATE_COMPARE_FN_H

#include "i_aggregate_fn.h"

#include <functional>

namespace dbcore::materialize
{
    class cmp_fn : public i_aggregate_fn
    {
        public:
            using compare_func = std::function<bool(const query::constant&, query::constant&)>;

        private:
            std::string m_fldname;
            std::string m_prefix;
            compare_func m_cmp_func;
            query::constant m_val;
            query::constant m_initial_val;

        public:
            explicit cmp_fn(const std::string& fldname,
                            const std::string& prefix,
                            compare_func cmp_func,
                            const query::constant& initial_val)
                : m_fldname(fldname)
                , m_prefix(prefix)
                , m_cmp_func(cmp_func)
                , m_initial_val(initial_val)
            {}

            void process_first(std::shared_ptr<scan::i_scan> s) override
            {
                m_val = s->get_val(m_fldname);
            }

            void process_next(std::shared_ptr<scan::i_scan> s) override
            {
                auto new_val = s->get_val(m_fldname);
                if (m_cmp_func(new_val, m_val))
                    m_val = new_val;
            }

            std::string field_name() const override
            {
                return m_prefix + m_fldname;
            }

            query::constant value() const override
            {
                return m_val;
            }
    };
}

#endif