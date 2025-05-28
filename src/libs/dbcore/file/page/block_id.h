#ifndef __FILE_MGR_BLOCKID_H
#define __FILE_MGR_BLOCKID_H

#include <functional>
#include <sstream>

namespace dbcore::file_mgr
{
    class block_id
    {
        private:
            std::string filename;
            int blk_num;

        public:
            block_id() : blk_num(0), filename("") {}
            explicit block_id(const std::string& new_filename, int new_blk_num)
                : filename(new_filename), blk_num(new_blk_num)
            {}

            std::string get_filename() const
            {
                return filename;
            }

            int get_block_number() const
            {
                return blk_num;
            }

            bool operator==(const block_id& other) const
            {
                return filename == other.get_filename() && blk_num == other.get_block_number();
            }

            std::string to_string() const
            {
                std::ostringstream oss;
                oss << "[file " << filename << ", block " << blk_num << "]";
                return oss.str();
            }

            struct hash
            {
                std::size_t operator()(const block_id& blk) const
                {
                    return std::hash<std::string>{}(blk.to_string());
                }
            };
    };
}

#endif