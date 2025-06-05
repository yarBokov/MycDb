#include "table_scan.h"
#include "libs/common/config.hpp"

namespace dbcore::record
{
    table_scan::table_scan(tx::transaction& tx, const std::string& table, const layout& layout)
        : m_tx(tx), m_layout(layout), m_filename(table + config::table_ext), m_curr_slot(-1)
    {
        if (m_tx.size(m_filename) == 0)
            move_to_new_block();
        else
            move_to_block(0);
    }

    //scan
    void table_scan::before_first()
    {
        move_to_block(0);
    }

    bool table_scan::next()
    {
        m_curr_slot = m_rec_page->next_after(m_curr_slot);
        while (m_curr_slot < 0)
        {
            if (at_last_block())
                return false;
            move_to_block(m_rec_page->block().get_block_number() + 1);
            m_curr_slot = m_rec_page->next_after(m_curr_slot);
        }
        return true;
    }

    int table_scan::get_int(const std::string& fldname)
    {
        return m_rec_page->get_int(m_curr_slot, fldname);
    }

    std::string table_scan::get_str(const std::string& fldname)
    {
        return m_rec_page->get_str(m_curr_slot, fldname);
    }

    query::constant table_scan::get_val(const std::string& fldname)
    {
        if (m_layout.get_schema().type(fldname) == schema::sql_types::integer)
            return query::constant(get_int(fldname));
        else
            return query::constant(get_str(fldname));
    }

    bool table_scan::has_field(const std::string& fldname)
    {
        return m_layout.get_schema().has_field(fldname);
    }

    void table_scan::close()
    {
        if (m_rec_page)
        {
            m_tx.unpin(m_rec_page->block());
            m_rec_page.reset();
        }
    }
    
    // update_scan
    void table_scan::set_val(const std::string& fldname, const query::constant& value)
    {
        if (m_layout.get_schema().type(fldname) == schema::sql_types::integer)
            set_int(fldname, value.as_int());
        else
            set_str(fldname, value.as_str());
    }

    void table_scan::set_int(const std::string& fldname, int value)
    {
        m_rec_page->set_int(m_curr_slot, fldname, value);
    }

    void table_scan::set_str(const std::string& fldname, const std::string& value)
    {
        m_rec_page->set_str(m_curr_slot, fldname, value);
    }

    void table_scan::insert()
    {
        m_curr_slot = m_rec_page->insert_after(m_curr_slot);
        while (m_curr_slot < 0)
        {
            if (at_last_block())
                move_to_new_block();
            else
                move_to_block(m_rec_page->block().get_block_number() + 1);
            m_curr_slot = m_rec_page->insert_after(m_curr_slot);
        }
    }

    void table_scan::delete_record()
    {
        m_rec_page->delete_record(m_curr_slot);
    }

    record::record_id table_scan::get_rid()
    {
        return record_id(m_rec_page->block().get_block_number(), m_curr_slot);
    }

    void table_scan::move_to_rid(const record::record_id& rid)
    {
        close();
        block_id blk(m_filename, rid.block_number());
        m_rec_page = std::make_unique<record_page>(m_tx, blk, m_layout);
        m_curr_slot = rid.slot(); 
    }

    void table_scan::move_to_block(int blk_num)
    {
        close();
        block_id blk(m_filename, blk_num);
        m_rec_page = std::make_unique<record_page>(m_tx, blk, m_layout);
        m_curr_slot = -1;
    }

    void table_scan::move_to_new_block()
    {
        close();
        auto blk = m_tx.append(m_filename);
        m_rec_page = std::make_unique<record_page>(m_tx, *blk, m_layout);
        m_rec_page->format();
        m_curr_slot = -1;
    }

    bool table_scan::at_last_block() const
    {
        return m_rec_page->block().get_block_number() == m_tx.size(m_filename) - 1;
    }
}