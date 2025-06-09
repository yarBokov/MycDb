#ifndef __RECORD_TABLESCAN_H
#define __RECORD_TABLESCAN_H

#include "record_page.h"
#include "layout.h"
#include "libs/dbcore/scan/i_update_scan.h"

#include <string>

namespace dbcore::record
{
    class table_scan : public scan::i_update_scan
    {
        public:
            table_scan(std::shared_ptr<tx::transaction> tx, const std::string& table, const layout& layout);
            //scan
            void before_first() override;
            bool next() override;
            int get_int(const std::string& fldname) override;
            std::string get_str(const std::string& fldname) override;
            query::constant get_val(const std::string& fldname) override;
            bool has_field(const std::string& fldname) override;
            void close() override;
            
            // update_scan
            void set_val(const std::string& fldname, const query::constant& val) override;
            void set_int(const std::string& fldname, int val) override;
            void set_str(const std::string& fldname, const std::string& val) override;
            void insert() override;
            void delete_record() override;
            record::record_id get_rid() override;
            void move_to_rid(const record::record_id& rid) override;

        private:
            std::shared_ptr<tx::transaction> m_tx;
            layout m_layout;
            std::string m_filename;
            std::unique_ptr<record_page> m_rec_page;
            int m_curr_slot;
 
            void move_to_block(int blk_num);
            void move_to_new_block();
            bool at_last_block() const;
    };
}

#endif