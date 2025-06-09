#ifndef __RECORD_RID_H
#define __RECORD_RID_H

#include <fmt/format.h>

namespace dbcore::record
{
    class record_id
    {
        private:
            int m_blk_num;
            int m_slot;

        public:
            record_id()
                : m_blk_num(-1), m_slot(-1)
            {}

            explicit record_id(int blk_num, int slot)
                :  m_blk_num(blk_num), m_slot(slot)
            {}

            int block_number() const
            {
                return m_blk_num;
            }

            int slot() const
            {
                return m_slot;
            }

            bool operator==(const record_id& other) const
            {
                return m_blk_num == other.m_blk_num && m_slot == other.m_slot;
            }

            std::string to_string() const
            {
                return fmt::format("[{}, {}]", m_blk_num, m_slot);
            }

            struct hash
            {
                std::size_t operator()(const record_id& rid) const
                {
                    return std::hash<int>()(rid.block_number()) ^ std::hash<int>()(rid.slot());
                }
            };
    };
}

#endif