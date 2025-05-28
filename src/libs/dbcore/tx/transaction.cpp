#include "transaction.h"

namespace dbcore::tx
{
    int transaction::next_tx = 0;

    std::mutex transaction::tx_num_mtx;

    transaction::transaction(file_mgr::file_mgr& fm, log_mgr::log_mgr& lm, buffer_mgr::buffer_mgr& bm)
        : fm(fm), bm(bm), buffers(bm), recovery_mgr(*this, next_tx_num(), lm, bm), tx_num(next_tx)
    {}

    void transaction::commit()
    {
        recovery_mgr.commit();
        // LOG_INFO_TX
        concurrency_mgr.release();
        buffers.unpin_all();
    }

    void transaction::rollback()
    {
        recovery_mgr.rollback();
        // LOG_INFO_TX
        concurrency_mgr.release();
        buffers.unpin_all();
    }

    void transaction::recover()
    {
        bm.flush_all(tx_num);
        recovery_mgr.recover();
    }
    
    void transaction::pin(const file_mgr::block_id& blk)
    {
        buffers.pin(blk);
    }

    void transaction::unpin(const file_mgr::block_id& blk)
    {
        buffers.unpin(blk);
    }
    
    int transaction::get_int(const file_mgr::block_id& blk, int offset)
    {
        concurrency_mgr.s_lock(blk);
        auto buf = buffers.get_buffer(blk);
        return buf->contents().get_int(offset);
    }

    std::string transaction::get_str(const file_mgr::block_id& blk, int offset)
    {
        concurrency_mgr.s_lock(blk);
        auto buf = buffers.get_buffer(blk);
        return buf->contents().get_string(offset);
    }
    
    void transaction::set_int(const file_mgr::block_id& blk, int offset, int value, bool ok_to_log)
    {
        concurrency_mgr.x_lock(blk);
        auto buf = buffers.get_buffer(blk);
        int lsn = -1;
        if (ok_to_log)
            lsn = recovery_mgr.set_int(*buf, offset, value);
        auto p = buf->contents();
        p.set_int(offset, value);
        buf->set_modified(tx_num, lsn);
    }

    void transaction::set_str(const file_mgr::block_id& blk, int offset, const std::string& value, bool ok_to_log)
    {
        concurrency_mgr.x_lock(blk);
        auto buf = buffers.get_buffer(blk);
        int lsn = -1;
        if (ok_to_log)
            lsn = recovery_mgr.set_string(*buf, offset, value);
        auto p = buf->contents();
        p.set_string(offset, value);
        buf->set_modified(tx_num, lsn);
    }
    
    int transaction::size(const std::string& filename)
    {
        file_mgr::block_id dummyblk(filename, npos);
        concurrency_mgr.s_lock(dummyblk);
        return fm.len(filename);
    }

    std::unique_ptr<file_mgr::block_id> transaction::append(const std::string& filename)
    {
        file_mgr::block_id dummyblk(filename, npos);
        concurrency_mgr.x_lock(dummyblk);
        return fm.append(filename);
    }
    
    int transaction::block_size() const
    {
        return fm.get_block_size();
    }

    int transaction::available_buffers_count() const
    {
        return bm.get_available_count();
    }
}