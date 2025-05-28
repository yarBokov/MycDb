#ifndef __QUERY_PREDICATE_H
#define __QUERY_PREDICATE_H

#include <vector>
#include <memory>
#include "term.h"

namespace dbcore::query
{
    class predicate
    {
        private:
            std::vector<term> terms;

        public:
            predicate() = default;
            predicate(const term& t)
            {
                terms.push_back(t);
            }
            
            void conjoin_with(const predicate& pred)
            {
                terms.insert(terms.end(), pred.terms.begin(), pred.terms.end());
            }

            bool is_satisfied(scan::i_scan& s) const
            {
                for (const auto& t : terms)
                    if (!t.is_satisfied(s))
                        return false;

                return true;
            }

            int calculate_reduction(const plan::plan& p) const
            {
                int factor = 1;
                for (const auto& t : terms)
                    factor *= t.calculate_reduction(p);
                return factor;
            }

            std::unique_ptr<predicate> select_sub_pred(const record::schema& sch) const
            {
                auto result = std::make_unique<predicate>();
                for (const auto& t : terms)
                    if (t.applies_to(sch))
                        result->terms.push_back(t);
                return result->terms.empty() ? nullptr : std::move(result);
            }

            std::unique_ptr<predicate> join_sub_pred(const record::schema& sch1, const record::schema& sch2) const
            {
                record::schema newsch;
                newsch.add_all(sch1);
                newsch.add_all(sch2);
                
                auto result = std::make_unique<predicate>();
                for (const auto& t : terms)
                    if (!t.applies_to(sch1) && !t.applies_to(sch2) && t.applies_to(newsch))
                        result->terms.push_back(t);
                return result->terms.empty() ? nullptr : std::move(result);
            }

            constant equates_with_constant(const std::string& fldname) const
            {
                for (const auto& t : terms) {
                    constant c = t.equates_with_constant(fldname);
                    if (c.as_int() != 0 || c.as_str() != "")
                        return c;
                }
                return constant(0);
            }

            std::string equates_with_field(const std::string& fldname) const
            {
                for (const auto& t : terms) {
                    std::string s = t.equates_with_field(fldname);
                    if (!s.empty())
                        return s;
                }
                return "";
            }

            std::string to_string() const
            {
                if (terms.empty()) return "";

                std::string result = terms[0].to_string();
                for (std::size_t i = 1; i < terms.size(); ++i)
                    result += " and " + terms[i].to_string();
                return result;
            }
    };
}

#endif