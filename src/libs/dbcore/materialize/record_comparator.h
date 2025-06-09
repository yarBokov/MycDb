#ifndef __MATERIALIZE_RECORD_COMPARATOR_H
#define __MATERIALIZE_RECORD_COMPARATOR_H

#include "libs/dbcore/scan/i_scan.h"

#include <vector>

namespace dbcore::materialize
{
    struct record_comparator
    {
        private:
            std::vector<std::string> m_fields;

        public:
            record_comparator(const std::vector<std::string>& fields)
                : m_fields(fields)
            {}

            int compare(const std::shared_ptr<scan::i_scan> s1, std::shared_ptr<scan::i_scan> s2) const
            {
                for (const auto& fldname : m_fields)
                {
                    if (s1->get_val(fldname) == s2->get_val(fldname))
                        return true;
                }
                return false;
            }
    };
}

#endif