#ifndef __METADATA_MD_MGR_H
#define __METADATA_MD_MGR_H

#include "table_mgr.h"
#include "view_mgr.h"
#include "stat_mgr.h"
#include "index_mgr.h"

#include "libs/dbcore/tx/transaction.h"

namespace dbcore::metadata
{
    class metadata_mgr
    {
        private:
            std::unique_ptr<table_mgr> m_tbl_mgr;
            std::unique_ptr<view_mgr> m_view_mgr;
            std::unique_ptr<stat_mgr> m_stat_mgr;
            std::unique_ptr<index_mgr> m_idx_mgr;

        public:
            metadata_mgr(bool is_new, tx::transaction& tx)
            {
                m_tbl_mgr = std::make_unique<table_mgr>(is_new, tx);
                m_view_mgr = std::make_unique<view_mgr>(is_new, tx);
                m_stat_mgr = std::make_unique<stat_mgr>(*m_tbl_mgr, tx);
                m_idx_mgr = std::make_unique<index_mgr>();
            }

            void create_table(const std::string& tblname, record::schema& sch, std::shared_ptr<tx::transaction> tx)
            {
                m_tbl_mgr->create_table(tblname, sch, tx);
            }

            std::unique_ptr<record::layout> get_layout(const std::string& tblname, std::shared_ptr<tx::transaction> tx)
            {
                return m_tbl_mgr->get_layout(tblname, tx);
            }

            void create_view(const std::string& vwname, std::string vwdef, std::shared_ptr<tx::transaction> tx)
            {
                m_view_mgr->create_view(vwname, vwdef, tx);
            }

            std::string get_view_def(const std::string& vwname, std::shared_ptr<tx::transaction> tx)
            {
                m_view_mgr->get_view_def(vwname, tx);
            }

            std::shared_ptr<stat_info> get_stat_info(const std::string& tblname, record::layout& layout, std::shared_ptr<tx::transaction> tx)
            {
                return m_stat_mgr->get_stat_info(tblname, layout, tx);
            }

            void create_index(const std::string& idxname, const std::string& tblname, const std::string& fldname, std::shared_ptr<tx::transaction> tx)
            {
                m_idx_mgr->create_index(idxname, tblname, fldname, tx);
            }

            std::unordered_map<std::string, index_info> get_index_info(const std::string& tblname, std::shared_ptr<tx::transaction> tx)
            {
                return m_idx_mgr->get_index_info(tblname, tx);
            }
        };
}

#endif