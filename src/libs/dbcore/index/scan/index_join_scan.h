#ifndef __INDEX_IDX_JOIN_SCAN_H
#define __INDEX_IDX_JOIN_SCAN_H

#include "libs/dbcore/scan/i_scan.h"
#include "libs/dbcore/index/i_index.h"

namespace dbcore::index
{
    class index_join_scan : public scan::i_scan
    {
        private:
            std::shared_ptr<scan::i_scan> m_lhs;
            std::shared_ptr<index::i_index> m_idx;
            std::string m_join_field;
            std::shared_ptr<record::table_scan> m_rhs;

            void reset_index()
            {
                auto searchkey = m_lhs->get_val(m_join_field);
                m_idx->before_first(searchkey);
            }

        public:
            index_join_scan(std::shared_ptr<scan::i_scan> lhs, std::shared_ptr<index::i_index> idx,
                const std::string& join_field, std::shared_ptr<record::table_scan> rhs)
                : m_lhs(lhs), m_idx(idx), m_join_field(join_field), m_rhs(rhs)
            {
                before_first();
            }

            ~index_join_scan() = default;

            void before_first() override
            {
                m_lhs->before_first();
                m_lhs->next();
                reset_index();
            }

            bool next() override
            {
                while (true)
                {
                    if (m_idx->next())
                    {
                        m_rhs->move_to_rid(m_idx->get_data_rid());
                        return true;
                    }
                    if (!m_lhs->next())
                        return false;
                    reset_index();
                }
            }

            int get_int(const std::string& fldname) override
            {
                if (m_rhs->has_field(fldname))
                    return m_rhs->get_int(fldname);
                else
                    return m_lhs->get_int(fldname);
            }

            std::string get_str(const std::string& fldname) override
            {
                if (m_rhs->has_field(fldname))
                    return m_rhs->get_str(fldname);
                else
                    return m_lhs->get_str(fldname);
            }

            query::constant get_val(const std::string& fldname) override
            {
                if (m_rhs->has_field(fldname))
                    return m_rhs->get_val(fldname);
                else
                    return m_lhs->get_val(fldname);
            }

            bool has_field(const std::string& fldname) override
            {
                return m_rhs->has_field(fldname) || m_lhs->has_field(fldname);
            }

            void close() override
            {
                m_lhs->close();
                m_idx->close();
                m_rhs->close();
            }
    };
};

#endif