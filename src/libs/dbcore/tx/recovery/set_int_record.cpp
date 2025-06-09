#include "set_int_record.h"

namespace dbcore::tx
{
    set_int_record::set_int_record(file_mgr::page& p)
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
        value = p.get_int(vpos);
    }

    log_operation set_int_record::operation() const { return log_operation::set_int; }
    int set_int_record::tx_num() const { return m_tx_num; }

    void set_int_record::undo(std::shared_ptr<transaction> tx) 
    {
        tx->pin(*blk);
        tx->set_int(*blk, offset, value, false);
        tx->unpin(*blk);
    }

    std::string set_int_record::to_string() const 
    { 
        return "<SETINT " + std::to_string(m_tx_num) + " " + blk->to_string()
            + " " + std::to_string(offset) + " " + std::to_string(value) + ">"; 
    }

    int set_int_record::write_to_log(log_mgr::log_mgr& lm, int tx_num, 
        const file_mgr::block_id& blk, int offset, int value)
    {
        int tpos = sizeof(int);
        int fpos = tpos + sizeof(int);
        int bpos = fpos + file_mgr::page::max_len(blk.get_filename().length());
        int opos = bpos + sizeof(int);
        int vpos = opos + sizeof(int);
        
        std::vector<char> rec(vpos + sizeof(int));
        file_mgr::page p(rec);
        
        p.set_int(0, log_operation_to_int(log_operation::set_int));
        p.set_int(tpos, tx_num);
        p.set_string(fpos, blk.get_filename());
        p.set_int(bpos, blk.get_block_number());
        p.set_int(opos, offset);
        p.set_int(vpos, value);
        
        return lm.append(rec);
    }
}