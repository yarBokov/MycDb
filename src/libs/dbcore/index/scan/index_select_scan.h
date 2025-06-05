#ifndef __INDEX_IDX_SELECT_SCAN_H
#define __INDEX_IDX_SELECT_SCAN_H

#include "libs/dbcore/scan/i_scan.h"
#include "libs/dbcore/index/i_index.h"
#include "libs/dbcore/record/table_scan.h"
#include "libs/dbcore/query/constant.h"

#include <memory>

namespace dbcore::index
{
    class index_select_scan : public scan::i_scan
    {
        private:
            std::shared_ptr<record::table_scan> m_ts;
            std::shared_ptr<i_index> m_idx;
            query::constant m_val;
            
        public:
            index_select_scan(std::shared_ptr<record::table_scan> ts, 
                std::shared_ptr<i_index> idx, const query::constant& val)
                : m_ts(ts), m_idx(idx), m_val(val)
            {
                before_first();
            }

            ~index_select_scan() = default;

            void before_first() override
            {
                m_idx->before_first(m_val);
            }

            bool next() override
            {
                bool ok = m_idx->next();
                if (ok)
                {
                    auto rid = m_idx->get_data_rid();
                    m_ts->move_to_rid(rid);
                }
                return ok;
            }

            int get_int(const std::string& fldname) override
            {
                return m_ts->get_int(fldname);
            }

            std::string get_str(const std::string& fldname) override
            {
                return m_ts->get_str(fldname);
            }

            query::constant get_val(const std::string& fldname) override
            {
                return m_ts->get_val(fldname);
            }

            bool has_field(const std::string& fldname) override
            {
                return m_ts->has_field(fldname);
            }

            void close() override
            {
                m_idx->close();
                m_ts->close();
            }
    };
}

#endif