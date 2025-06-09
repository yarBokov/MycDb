#ifndef __METADATA_VIEW_MGR_H
#define __METADATA_VIEW_MGR_H

#include "table_mgr.h"
#include "libs/dbcore/tx/transaction.h"

namespace dbcore::metadata
{
    class view_mgr
    {
        private:
            static const int MAX_VIEWDEF = 100;
            table_mgr& m_tbl_mgr;
        
        public:
            view_mgr(bool is_new, table_mgr& tbl_mgr, std::shared_ptr<tx::transaction> tx)
                : m_tbl_mgr(tbl_mgr)
            {
                if (is_new)
                {
                    record::schema sch;
                    sch.add_str_field(detail::viewname_field, table_mgr::MAX_NAME);
                    sch.add_str_field(detail::viewdef_field, MAX_VIEWDEF);
                    m_tbl_mgr.create_table(detail::view_catalog_tbl, sch, tx);
                }
            }

            void create_view(const std::string& vw_name, const std::string& vw_def, std::shared_ptr<tx::transaction> tx)
            {
                auto layout = m_tbl_mgr.get_layout(detail::fields_catalog_tbl, tx);
                record::table_scan tbl_sc(tx, detail::view_catalog_tbl, *layout);
                tbl_sc.insert();
                tbl_sc.set_str(detail::viewname_field, vw_name);
                tbl_sc.set_str(detail::viewdef_field, vw_def);
                tbl_sc.close();
            }

            std::string get_view_def(const std::string& vw_name, std::shared_ptr<tx::transaction> tx)
            {
                std::string result;
                auto layout = m_tbl_mgr.get_layout(detail::view_catalog_tbl, tx);
                record::table_scan tbl_sc(tx, detail::view_catalog_tbl, *layout);
                while (tbl_sc.next())
                {
                    if (tbl_sc.get_str(detail::viewname_field) == vw_name)
                    {
                        result = tbl_sc.get_str(detail::viewdef_field);
                        break;
                    }
                }
                tbl_sc.close();
                return result;
            }
    };
}

#endif