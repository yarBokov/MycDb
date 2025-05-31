#ifndef __METADATA_IDX_MGR_H
#define __METADATA_IDX_MGR_H

#include "libs/dbcore/record/layout.h"
#include "libs/dbcore/record/schema.h"
#include "libs/dbcore/record/table_scan.h"
#include "index_info.h"
#include "table_mgr.h"
#include "stat_mgr.h"
#include "constants.h"

#include <unordered_map>

namespace dbcore::metadata
{
    class index_mgr
    {
        private:
            std::unique_ptr<record::layout> m_idxcat_layout;
            std::unique_ptr<table_mgr> m_tbl_mgr;
            std::unique_ptr<stat_mgr> m_stat_mgr;

        public:
            index_mgr(bool is_new, std::unique_ptr<table_mgr> tbl_mgr, std::unique_ptr<stat_mgr> stat_mgr, tx::transaction& tx)
                : m_tbl_mgr(std::move(tbl_mgr)), m_stat_mgr(std::move(stat_mgr))
            {
                using namespace detail;
                if (is_new)
                {
                    record::schema idx_sch;
                    idx_sch.add_str_field(idxname_field, MAX_NAME);
                    idx_sch.add_str_field(tblname_field, MAX_NAME);
                    idx_sch.add_str_field(fldname_field, MAX_NAME);
                    m_tbl_mgr->create_table(index_catalog_tbl, idx_sch, tx);
                }
                m_idxcat_layout = m_tbl_mgr->get_layout(index_catalog_tbl, tx);
            }

            std::unordered_map<std::string, index_info> get_index_info(const std::string& tblname, tx::transaction& tx)
            {
                record::table_scan ts(tx, detail::index_catalog_tbl,*m_idxcat_layout);
                std::unordered_map<std::string, index_info> result;
                while (ts.next())
                {
                    if (ts.get_str(detail::tblname_field) == tblname)
                    {
                        std::string idxname = ts.get_str(detail::idxname_field);
                        std::string fldname = ts.get_str(detail::fldname_field);
                        auto tbl_layout = m_tbl_mgr->get_layout(tblname, tx);
                        auto tbl_stats = m_stat_mgr->get_stat_info(tblname, *tbl_layout, tx);
                        index_info idx_info(idxname, fldname, 
                            std::make_unique<record::schema>(tbl_layout->get_schema()), tx, *tbl_stats);
                        result.insert_or_assign(fldname, idx_info);
                    }
                }
                ts.close();
                return result;
            }

            void create_index(const std::string& idxname, const std::string& tblname, const std::string& fldname, tx::transaction& tx)
            {
                record::table_scan ts(tx, detail::index_catalog_tbl, *m_idxcat_layout);
                ts.insert();
                ts.set_str(detail::idxname_field, idxname);
                ts.set_str(detail::tblname_field, tblname);
                ts.set_str(detail::fldname_field, fldname);
                ts.close();
            }
    };
}

#endif