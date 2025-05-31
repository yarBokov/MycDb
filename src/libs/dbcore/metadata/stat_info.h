#ifndef __METADATA_STAT_INFO_H
#define __METADATA_STAT_INFO_H

#include <string>

namespace dbcore::metadata
{
    class stat_info
    {
        private:
            int m_num_blocks;
            int m_num_recs;
            
        public:
            stat_info() : m_num_blocks(0), m_num_recs(0) {}
            stat_info(int num_blocks, int num_recs)
                : m_num_blocks(num_blocks), m_num_recs(num_recs)
            {}
            
            std::size_t blocks_accessed() const
            {
                return m_num_blocks;
            }

            std::size_t records_output() const
            {
                return m_num_recs;
            }

            std::size_t distinct_values(const std::string& fldname) const
            {
                return 1 + (m_num_recs / 3); //TODO: make useful calculation later
            }
    };
}

#endif