#include "transaction.h"

namespace dbcore::tx
{
    int transaction::m_next_tx = 0;

    std::mutex transaction::tx_num_mtx;

    transaction::transaction(file_mgr::file_mgr& fm, log_mgr::log_mgr& lm, buffer_mgr::buffer_mgr& bm)
        : m_fm(fm), m_bm(bm), m_buffers(bm), m_recovery_mgr(this, next_tx_num(), lm, bm), m_tx_num(m_next_tx)
    {}

    void transaction::commit()
    {
        m_recovery_mgr.commit();
        // LOG_INFO_TX
        m_concurrency_mgr.release();
        m_buffers.unpin_all();
    }

    void transaction::rollback()
    {
        m_recovery_mgr.rollback();
        // LOG_INFO_TX
        m_concurrency_mgr.release();
        m_buffers.unpin_all();
    }

    void transaction::recover()
    {
        m_bm.flush_all(m_tx_num);
        m_recovery_mgr.recover();
    }
    
    void transaction::pin(const file_mgr::block_id& blk)
    {
        m_buffers.pin(blk);
    }

    void transaction::unpin(const file_mgr::block_id& blk)
    {
        m_buffers.unpin(blk);
    }
    
    int transaction::get_int(const file_mgr::block_id& blk, int offset)
    {
        m_concurrency_mgr.s_lock(blk);
        auto buf = m_buffers.get_buffer(blk);
        return buf->contents().get_int(offset);
    }

    std::string transaction::get_str(const file_mgr::block_id& blk, int offset)
    {
        m_concurrency_mgr.s_lock(blk);
        auto buf = m_buffers.get_buffer(blk);
        return buf->contents().get_string(offset);
    }
    
    void transaction::set_int(const file_mgr::block_id& blk, int offset, int value, bool ok_to_log)
    {
        m_concurrency_mgr.x_lock(blk);
        auto buf = m_buffers.get_buffer(blk);
        int lsn = -1;
        if (ok_to_log)
            lsn = m_recovery_mgr.set_int(*buf, offset, value);
        auto p = buf->contents();
        p.set_int(offset, value);
        buf->set_modified(m_tx_num, lsn);
    }

    void transaction::set_str(const file_mgr::block_id& blk, int offset, const std::string& value, bool ok_to_log)
    {
        m_concurrency_mgr.x_lock(blk);
        auto buf = m_buffers.get_buffer(blk);
        int lsn = -1;
        if (ok_to_log)
            lsn = m_recovery_mgr.set_string(*buf, offset, value);
        auto p = buf->contents();
        p.set_string(offset, value);
        buf->set_modified(m_tx_num, lsn);
    }
    
    int transaction::size(const std::string& filename)
    {
        file_mgr::block_id dummyblk(filename, npos);
        m_concurrency_mgr.s_lock(dummyblk);
        return m_fm.len(filename);
    }

    std::unique_ptr<file_mgr::block_id> transaction::append(const std::string& filename)
    {
        file_mgr::block_id dummyblk(filename, npos);
        m_concurrency_mgr.x_lock(dummyblk);
        return m_fm.append(filename);
    }
    
    int transaction::block_size() const
    {
        return m_fm.get_block_size();
    }

    int transaction::available_buffers_count() const
    {
        return m_bm.get_available_count();
    }
}