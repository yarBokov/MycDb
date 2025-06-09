#include "set_str_record.h"

namespace dbcore::tx
{
    set_str_record::set_str_record(file_mgr::page& p)
    {
        int tpos = sizeof(int);
        m_tx_num = p.get_int(tpos);
        
        int fpos = tpos + sizeof(int);
        std::string filename = p.get_string(fpos);
        
        int bpos = fpos + file_mgr::page::max_len(filename.length());
        int blknum = p.get_int(bpos);
        blk = std::make_unique<file_mgr::block_id>(filename, blknum);
        
        int opos = bpos + sizeof(int);
        offset = p.get_int(opos);
        
        int vpos = opos + sizeof(int);
        value = p.get_string(vpos);
    }

    log_operation set_str_record::operation() const { return log_operation::set_str; }
    int set_str_record::tx_num() const { return m_tx_num; }

    void set_str_record::undo(std::shared_ptr<transaction> tx) 
    {
        tx->pin(*blk);
        tx->set_str(*blk, offset, value, false);
        tx->unpin(*blk);
    }

    std::string set_str_record::to_string() const 
    { 
        return "<SETSTR " + std::to_string(m_tx_num) + " " + blk->to_string()
            + " " + std::to_string(offset) + " " + value + ">"; 
    }

    int set_str_record::write_to_log(log_mgr::log_mgr& lm, int tx_number, 
        const file_mgr::block_id& blk, int offset, const std::string& value)
    {
        int tpos = sizeof(int);
        int fpos = tpos + sizeof(int);
        int bpos = fpos + file_mgr::page::max_len(blk.get_filename().length());
        int opos = bpos + sizeof(int);
        int vpos = opos + sizeof(int);
        int rec_len = vpos + file_mgr::page::max_len(value.length());
        
        std::vector<char> rec(rec_len);
        file_mgr::page p(rec);
        
        p.set_int(0, log_operation_to_int(log_operation::set_str));
        p.set_int(tpos, tx_number);
        p.set_string(fpos, blk.get_filename());
        p.set_int(bpos, blk.get_block_number());
        p.set_int(opos, offset);
        p.set_string(vpos, value);
        
        return lm.append(rec);
    }
}