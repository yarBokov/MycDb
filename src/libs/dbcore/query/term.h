#ifndef __QUERY_TERM_H
#define __QUERY_TERM_H

#include "expression.h"
#include "plan/plan.h"
#include <limits>

namespace dbcore::query
{
    class term
    {
        private:
            expression m_lhs;
            expression m_rhs;

        public:
            term(const expression& lhs, const expression& rhs)
                : m_lhs(lhs), m_rhs(rhs)
            {}

            bool is_satisfied(scan::scan& s) const
            {
                auto lhsval = m_lhs.evaluate(s);
                auto rhsval = m_rhs.evaluate(s);
                return lhsval == rhsval;
            }

            int calculate_reduction(const plan::plan& p) const
            {
                if (m_lhs.is_field_name() && m_rhs.is_field_name()) {
                    std::string lhs_name = m_lhs.as_field_name();
                    std::string rhs_mame = m_rhs.as_field_name();
                    return std::max(p.distinct_values(lhs_name), p.distinct_values(rhs_mame));
                }
                if (m_lhs.is_field_name())
                    return p.distinct_values(m_lhs.as_field_name());
                if (m_rhs.is_field_name())
                    return p.distinct_values(m_rhs.as_field_name());
                return m_lhs.as_constant() == m_rhs.as_constant() ? 1 : std::numeric_limits<int>::max();
            }

            constant equates_with_constant(const std::string& fldname) const
            {
                if (m_lhs.is_field_name() && m_lhs.as_field_name() == fldname && !m_rhs.is_field_name())
                    return m_rhs.as_constant();
                if (m_rhs.is_field_name() && m_rhs.as_field_name() == fldname && !m_lhs.is_field_name())
                    return m_lhs.as_constant();
                return constant(0);
            }

            std::string equates_with_field(const std::string& fldname) const
            {
                if (m_lhs.is_field_name() && m_lhs.as_field_name() == fldname && m_rhs.is_field_name())
                    return m_rhs.as_field_name();
                if (m_rhs.is_field_name() && m_rhs.as_field_name() == fldname && m_lhs.is_field_name())
                    return m_lhs.as_field_name();
                return "";
            }

            bool applies_to(const record::schema& sch) const
            {
                return m_lhs.applies_to(sch) && m_rhs.applies_to(sch);
            }

            std::string to_string() const
            {
                return m_lhs.to_string() + "=" + m_rhs.to_string();
            }
    };
}

#endif