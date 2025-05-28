#ifndef __METADATA_STAT_MGR_H
#define __METADATA_STAT_MGR_H

#include "stat_info.h"
#include "table_mgr.h"

#include <unordered_map>
#include <memory>

namespace dbcore::metadata
{
    class stat_mgr
    {
        private:
            table_mgr& m_tbl_mgr;
            std::unordered_map<std::string, std::shared_ptr<stat_info>> m_tbl_stats;
            std::size_t m_num_calls_limit;
            int m_num_calls;

            std::shared_ptr<stat_info> calculate_table_stats(
                const std::string& tbl_name, const record::layout& layout, tx::transaction& tx)
            {
                int num_recs = 0;
                int num_blocks = 0;
                record::table_scan tbl_sc(tx, tbl_name, layout);
                while (tbl_sc.next())
                {
                    num_recs++;
                    num_blocks = tbl_sc.get_rid().block_number() + 1;
                }

                tbl_sc.close();
                return std::make_shared<stat_info>(num_blocks, num_recs);
            }

            void refresh_statistics(tx::transaction& tx)
            {
                m_tbl_stats.clear();
                m_num_calls = 0;
                
                auto tcat_layout = m_tbl_mgr.get_layout(detail::table_catalog_tbl, tx);
                record::table_scan tbl_sc(tx, detail::table_catalog_tbl, *tcat_layout);
                while (tbl_sc.next())
                {
                    std::string tblname = tbl_sc.get_str(detail::tblname_field);
                    auto layout = m_tbl_mgr.get_layout(tblname, tx);
                    m_tbl_stats[tblname] = calculate_table_stats(tblname, *layout, tx);
                }
                tbl_sc.close();
            }

        public:
            stat_mgr(table_mgr& tbl_mgr, tx::transaction& tx)
                : m_tbl_mgr(tbl_mgr), m_num_calls(0), m_num_calls_limit(100)
            {
                refresh_statistics(tx);
            }

            std::shared_ptr<stat_info> stat_mgr::get_stat_info(
                const std::string& tbl_name, const record::layout& layout, tx::transaction& tx)
            {
                m_num_calls++;
                if (m_num_calls > m_num_calls_limit)
                    refresh_statistics(tx);

                auto it = m_tbl_stats.find(tbl_name);
                if (it == m_tbl_stats.end())
                {
                    auto si = calculate_table_stats(tbl_name, layout, tx);
                    m_tbl_stats[tbl_name] = std::move(si);
                    return m_tbl_stats[tbl_name];
                }

                return it->second;
            }

            void set_num_calls_limit(int num_calls_limit)
            {
                m_num_calls_limit = num_calls_limit;
            }

            int get_current_num_calls_limit()
            {
                return m_num_calls_limit;
            }
    };
}

#endif