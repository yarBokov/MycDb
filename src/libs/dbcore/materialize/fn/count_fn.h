#ifndef __MATERIALIZE_AGGREGATE_COUNT_FN_H
#define __MATERIALIZE_AGGREGATE_COUNT_FN_H

#include "i_aggregate_fn.h"

namespace dbcore::materialize
{
    class count_fn : public i_aggregate_fn
    {
        private:
            std::string m_fldname;
            int m_count;
        
        public:
            explicit count_fn(const std::string& fldname)
                : m_fldname(fldname), m_count(0)
            {}

            void process_first(std::shared_ptr<scan::i_scan> s) override
            {
                m_count = 1;
            }

            void process_next(std::shared_ptr<scan::i_scan> s) override
            {
                m_count++;
            }

            std::string field_name() const override
            {
                return field_prefixes::count_fn_prefix + m_fldname;
            }

            query::constant value() const override
            {
                return query::constant(m_count);
            }
    };
}

#endif