#ifndef __MULTIBUFFER_PRODUCT_SCAN_H
#define __MULTIBUFFER_PRODUCT_SCAN_H

#include "chunk_scan.h"
#include "libs/dbcore/scan/product_scan.h"

#include "libs/common/config.hpp"

#include "buffer_needs.h"

using namespace dbcore::scan;

namespace dbcore::multibuffer
{
    class multibuffer_product_scan : public i_scan
    {
        private:
            std::shared_ptr<tx::transaction> m_tx;
            std::shared_ptr<i_scan> m_lhs_scan;
            std::shared_ptr<i_scan> m_rhs_scan;
            std::shared_ptr<i_scan> m_prod_scan;
            std::string m_filename;
            record::layout m_layout;
            int m_chunk_size;
            int m_next_blk_num;
            int m_file_size;

            bool use_next_chunk()
            {
                if (m_next_blk_num >= m_file_size)
                    return false;

                if (m_rhs_scan)
                    m_rhs_scan->close();

                int end = m_next_blk_num + m_chunk_size - 1;
                if (end >= m_file_size)
                    end = m_file_size - 1;

                m_rhs_scan = std::make_shared<chunk_scan>(m_tx, m_filename, m_layout, m_next_blk_num, end);
                m_lhs_scan->before_first();
                m_prod_scan = std::make_shared<scan::product_scan>(m_lhs_scan, m_rhs_scan);
                m_next_blk_num = end + 1;
                return true;
            }

        public:
            multibuffer_product_scan(
                std::shared_ptr<tx::transaction> tx,
                std::shared_ptr<i_scan> lhs_scan,
                const std::string& tblname,
                const record::layout& layout)
                : m_tx(tx)
                , m_lhs_scan(lhs_scan)
                , m_filename(tblname + config::table_ext)
                , m_layout(layout)
            {
                m_file_size = m_tx->size(m_filename);
                int available = m_tx->available_buffers_count();
                m_chunk_size = buffer_needs::best_factor(available, m_file_size);
                before_first();
            }

            void before_first() override
            {
                m_next_blk_num = 0;
                use_next_chunk();
            }

            bool next() override
            {
                while (!m_prod_scan->next())
                {
                    if (!use_next_chunk())
                        return false;
                }
                return true;
            }

            int get_int(const std::string& fldname) override
            {
                return m_prod_scan->get_int(fldname);
            }

            std::string get_str(const std::string& fldname) override
            {
                return m_prod_scan->get_str(fldname);
            }

            query::constant get_val(const std::string& fldname) override
            {
                return m_prod_scan->get_val(fldname);
            }

            bool has_field(const std::string& fldname) override
            {
                return m_prod_scan->has_field(fldname);
            }

            void close() override
            {
                if (m_prod_scan)
                    m_prod_scan->close();
            }
    };
}

#endif