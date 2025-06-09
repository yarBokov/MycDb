#ifndef __MATERIALIZE_GROUP_VALUE_H
#define __MATERIALIZE_GROUP_VALUE_H

#include "libs/dbcore/tx/transaction.h"
#include "libs/dbcore/query/constant.h"
#include "libs/dbcore/scan/i_scan.h"

#include <vector>
#include <map>

namespace dbcore::materialize
{
    class group_value
    {
        private:
            std::map<std::string, query::constant> m_vals;

        public:
            group_value(std::shared_ptr<scan::i_scan> s, const std::vector<std::string>& fields)
            {
                for (const auto& fldname : fields)
                {
                    m_vals[fldname] = s->get_val(fldname);
                }
            }

            query::constant get_val(const std::string& fldname) const
            {
                return m_vals.at(fldname);
            }

            bool equals(const std::shared_ptr<group_value>& other) const
            {
                for (const auto& [fldname, val] : m_vals)
                {
                    if (!(val == other->get_val(fldname)))
                        return false;
                }
                return true;
            }
    };
}

#endif