#include "buffer_mgr.h"

namespace dbcore::buffer_mgr
{
    buffer_mgr::buffer_mgr(std::shared_ptr<file_mgr::file_mgr> fm, 
        std::shared_ptr<log_mgr::log_mgr> lm, int num_bufs)
        : num_available(num_bufs)
    {
        for (int i = 0; i < num_bufs; i++)
            buffer_pool.push_back(std::make_unique<buffer>(fm ,lm));
    }

    int buffer_mgr::get_available_count()
    {
        std::lock_guard<std::mutex> lock(buffer_mtx);
        return num_available;
    }

    void buffer_mgr::flush_all(int tx_num)
    {
        std::lock_guard<std::mutex> lock(buffer_mtx);
        for (auto& buf: buffer_pool)
            if (buf->modifying_tx() == tx_num)
                buf->flush();
    }

    void buffer_mgr::unpin(buffer& buf)
    {
        std::unique_lock<std::mutex> lock(buffer_mtx);
        buf.unpin();
        if (!buf.is_pinned())
        {
            num_available++;
            buffer_available.notify_all();
        }
    }

    buffer& buffer_mgr::pin(const block_id& blk)
    {
        std::unique_lock<std::mutex> lock(buffer_mtx);
        auto start = std::chrono::steady_clock::now();

        buffer* buf = try_pin(blk);
        while (buf == nullptr && !timeout_expired(start))
        {
            buffer_available.wait_for(lock, MAX_TIMEOUT);
            buf = try_pin(blk);
        }

        if (buf == nullptr)
            throw exceptions::buffer_abort_exception();
        
        return *buf;
    }

    bool buffer_mgr::timeout_expired(
        const std::chrono::steady_clock::time_point& start_time) const
    {
        return std::chrono::steady_clock::now() - start_time > MAX_TIMEOUT;
    }

    buffer* buffer_mgr::try_pin(const block_id& blk)
    {
        buffer* buf = find_existing_buffer(blk);
        if (buf == nullptr)
        {
            buf = choose_unpinned_buffer();
            if (buf == nullptr)
                return nullptr;

            buf->assign_to_block(blk);
        }

        if (!buf->is_pinned())
            num_available--;

        buf->pin();
        return buf;
    }

    buffer* buffer_mgr::find_existing_buffer(const block_id& blk)
    {
        for (auto& buf : buffer_pool)
        {
            auto b = buf->block();
            if (b && *b == blk)
                return buf.get();
        }
        return nullptr;
    }

    buffer* buffer_mgr::choose_unpinned_buffer()
    {
        for (auto& buf : buffer_pool)
        {
            if (!buf->is_pinned())
                return buf.get();
        }
        return nullptr;
    }
}