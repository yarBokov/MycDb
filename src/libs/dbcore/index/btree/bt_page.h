#ifndef __INDEX_BTREE_IDX_PAGE_H
#define __INDEX_BTREE_IDX_PAGE_H

#include "libs/dbcore/tx/transaction.h"
#include "libs/dbcore/file/page/block_id.h"
#include "libs/dbcore/record/layout.h"
#include "libs/dbcore/query/constant.h"
#include "libs/dbcore/record/record_id.h"

#include <memory>

namespace dbcore::index
{
    class btree_page
    {
        private:
            std::shared_ptr<tx::transaction> m_tx;
            file_mgr::block_id m_curr_blk;
            record::layout m_layout;

            int get_int(int slot, const std::string& fldname) const
            {
                int pos = fld_pos(slot, fldname);
                return m_tx->get_int(m_curr_blk, pos);
            }

            std::string get_string(int slot, const std::string& fldname) const
            {
                int pos = fld_pos(slot, fldname);
                return m_tx->get_str(m_curr_blk, pos);
            }

            query::constant get_val(int slot, const std::string& fldname) const
            {
                auto type = m_layout.get_schema()->type(fldname);
                if (type == dbcore::record::schema::sql_types::integer)
                    return query::constant(get_int(slot, fldname));
                else
                    return query::constant(get_string(slot, fldname));
            }

            void set_int(int slot, const std::string& fldname, int val)
            {
                int pos = fld_pos(slot, fldname);
                m_tx->set_int(m_curr_blk, pos, val, true);
            }

            void set_string(int slot, const std::string& fldname, const std::string& val)
            {
                int pos = fld_pos(slot, fldname);
                m_tx->set_str(m_curr_blk, pos, val, true);
            }

            void set_val(int slot, const std::string& fldname, const query::constant& val)
            {
                auto type = m_layout.get_schema()->type(fldname);
                if (type == dbcore::record::schema::sql_types::integer)
                    set_int(slot, fldname, val.as_int());
                else
                    set_string(slot, fldname, val.as_str());
            }

            void set_num_recs(int n)
            {
                m_tx->set_int(m_curr_blk, sizeof(int), n, true);
            }

            void insert(int slot)
            {
                for (int i = get_num_recs(); i > slot; i--)
                    copy_record(i - 1, i);
                set_num_recs(get_num_recs() + 1);
            }

            void copy_record(int from, int to)
            {
                const auto& sch = m_layout.get_schema();
                for (const auto& fldname : sch->fields())
                    set_val(to, fldname, get_val(from ,fldname));
            }

            void transfer_recs(int slot, btree_page& dest)
            {
                int dest_slot = 0;
                while (slot < get_num_recs())
                {
                    dest.insert(dest_slot);
                    const auto& sch = m_layout.get_schema();
                    for (const auto& fldname : sch->fields())
                        dest.set_val(dest_slot, fldname, get_val(slot, fldname));

                    delete_record(slot);
                    dest_slot++;
                }
            }

            int fld_pos(int slot, const std::string& fldname) const
            {
                int offset = m_layout.offset(fldname);
                return slot_pos(slot) + offset;
            }

            int slot_pos(int slot) const
            {
                int slot_size = m_layout.slot_size();
                return sizeof(int) + sizeof(int) + (slot * slot_size);
            }

            void make_default_record(const block_id& blk, int pos)
            {
                for (const auto& fldname : m_layout.get_schema()->fields())
                {
                    int offset = m_layout.offset(fldname);
                    if (m_layout.get_schema()->type(fldname) == dbcore::record::schema::sql_types::integer)
                        m_tx->set_int(blk, pos + offset, 0, false);
                    else
                        m_tx->set_str(blk ,pos + offset, "", false);
                }
            }

        public:
            btree_page(std::shared_ptr<tx::transaction> tx, const file_mgr::block_id& blk, const record::layout& layout)
                : m_tx(tx), m_curr_blk(blk), m_layout(layout)
            {}

            btree_page& operator=(const btree_page& other)
            {
                m_tx = other.m_tx;
                m_curr_blk = other.m_curr_blk;
                m_layout = other.m_layout;
                return *this;
            }

            ~btree_page()
            {
                close();
            }

            int find_slot_before(const query::constant& searchkey)
            {
                int slot = 0;
                while (slot < get_num_recs() && get_data_val(slot) != searchkey)
                    slot++;
                return slot - 1;
            }

            void close()
            {
                if (m_curr_blk.is_valid())
                {
                    m_tx->unpin(m_curr_blk);
                    m_curr_blk = file_mgr::block_id();
                }
            }

            bool is_full() const
            {
                return slot_pos(get_num_recs() + 1) >= m_tx->block_size();
            }

            file_mgr::block_id split(int split_pos, int flag)
            {
                auto new_blk = append_new(flag);
                btree_page new_page(m_tx, new_blk, m_layout);
                transfer_recs(split_pos, new_page);
                new_page.set_flag(flag);
                new_page.close();
                return new_blk;
            }

            query::constant get_data_val(int slot) const
            {
                return get_val(slot, "dataval");
            }

            int get_flag() const
            {
                return m_tx->get_int(m_curr_blk, 0);
            }

            void set_flag(int val)
            {
                m_tx->set_int(m_curr_blk, 0, val, true);
            }

            block_id append_new(int flag)
            {
                auto blk = m_tx->append(m_curr_blk.get_filename());
                m_tx->pin(*blk);
                format(*blk, flag);
                return *blk;
            }

            void format(const block_id& blk, int flag)
            {
                m_tx->set_int(blk, 0, flag, false);
                m_tx->set_int(blk, sizeof(int), 0, false);
                int rec_size = m_layout.slot_size();
                for (int pos = 2 * sizeof(int); pos + rec_size <= m_tx->block_size(); pos += rec_size)
                    make_default_record(blk, pos);
            }

            int get_child_num(int slot) const
            {
                return get_int(slot, "block");
            }

            void insert_dir(int slot, const  query::constant& val, int blk_num)
            {
                insert(slot);
                set_val(slot, "dataval", val);
                set_val(slot, "block", blk_num);
            }

            record::record_id get_data_rid(int slot) const
            {
                return record::record_id(get_int(slot, "block"), get_int(slot, "id"));
            }

            void insert_leaf(int slot, const  query::constant& val, const record::record_id& rid)
            {
                insert(slot);
                set_val(slot, "dataval", val);
                set_val(slot, "block", rid.block_number());
                set_val(slot, "id", rid.slot());
            }

            void delete_record(int slot)
            {
                for (int i = slot + 1; i < get_num_recs(); i++)
                    copy_record(i, i - 1);
                set_num_recs(get_num_recs() - 1);
            }

            int get_num_recs() const
            {
                return m_tx->get_int(m_curr_blk, sizeof(int));
            }
    };
}

#endif