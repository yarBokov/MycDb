#ifndef __TX_SET_INT_RECORD_H
#define __TX_SET_INT_RECORD_H

#include "log_record.h"
#include "libs/dbcore/log/log_mgr.h"

namespace dbcore::tx
{
    class set_int_record : public log_record
    {
        private: 
            int tx_num;
            int offset;
            int value;
            file_mgr::block_id& blk;
        public:
            set_int_record() = default;
            explicit set_int_record(file_mgr::page& p)
            {
                int tpos = sizeof(int);
                tx_num = p.get_int(tpos);
                
                int fpos = tpos + sizeof(int);
                std::string filename = p.get_string(fpos);
                
                int bpos = fpos + file_mgr::page::max_len(filename.length());
                int blknum = p.get_int(bpos);
                blk = file_mgr::block_id(filename, blknum);
                
                int opos = bpos + sizeof(int);
                offset = p.get_int(opos);
                
                int vpos = opos + sizeof(int);
                value = p.get_int(vpos);
            }
        
            log_operation operation() const override { return log_operation::set_int; }
            int tx_num() const override { return tx_num; }

            void undo(transaction& tx) override 
            {
                tx.pin(blk);
                tx.set_int(blk, offset, value, false);
                tx.unpin(blk);
            }

            std::string to_string() const override 
            { 
                return "<SETINT " + std::to_string(tx_num) + " " + blk.to_string()
                    + " " + std::to_string(offset) + " " + std::to_string(value) + ">"; 
            }

            static int write_to_log(log_mgr::log_mgr& lm, int tx_num, 
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
    };
}

#endif