#ifndef __QUERY_EXPRESSION_H
#define __QUERY_EXPRESSION_H

#include <memory>
#include "constant.h"
#include "libs/dbcore/scan/i_scan.h"
#include "libs/dbcore/record/schema.h"

namespace dbcore::query
{
    class expression
    {
        private:
            std::unique_ptr<constant> m_val;
            std::unique_ptr<std::string> m_fldname;

        public:
            expression(const constant& val) : m_val(std::make_unique<constant>(val)) {}
            expression(const std::string& fldname)
                : m_fldname(std::make_unique<std::string>(fldname))
            {}

            expression(const expression& other)
            {
                if (other.m_fldname)
                    this->m_fldname = std::make_unique<std::string>(*other.m_fldname);
                else if (other.m_val)
                    this->m_val = std::make_unique<constant>(*other.m_val);
            }

            expression& operator=(const expression& other)
            {
                if (this != &other)
                {
                    if (other.m_val) {
                        m_val = std::make_unique<constant>(*other.m_val);
                        m_fldname.reset();
                    } else if (other.m_fldname) {
                        m_fldname = std::make_unique<std::string>(*other.m_fldname);
                        m_val.reset();
                    } else {
                        m_val.reset();
                        m_fldname.reset();
                    }
                }
                return *this;
            }

            expression(expression&& other)
                : m_fldname(std::move(other.m_fldname)), m_val(std::move(other.m_val))
            {}

            expression& operator=(expression&& other)
            {
                if (this != &other)
                {
                    m_val = std::move(other.m_val);
                    m_fldname = std::move(other.m_fldname);
                }
                return *this;
            }

            ~expression() = default;
            
            constant evaluate(scan::i_scan& s) const
            {
                return m_val ? *m_val : s.get_val(*m_fldname);
            }

            bool is_field_name() const
            {
                return m_fldname != nullptr;
            }

            constant as_constant() const
            {
                return m_val ? *m_val : throw std::runtime_error("Not a constant expression: " + m_val->to_string());
            }

            std::string as_field_name() const
            {
                return m_fldname ? *m_fldname : throw std::runtime_error("Not a field name expression: " + *m_fldname);
            }

            bool applies_to(const record::schema& sch) const
            {
                return m_val ? true : sch.has_field(*m_fldname);
            }

            bool applies_to(std::shared_ptr<record::schema> sch) const
            {
                return m_val ? true : sch->has_field(*m_fldname);
            }

            std::string to_string() const
            {
                return m_val ? m_val->to_string() : *m_fldname;
            }
    };
}

#endif