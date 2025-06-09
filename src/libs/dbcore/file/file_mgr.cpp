#include "file_mgr.h"

#include "libs/common/config.hpp"
#include <fmt/format.h>

namespace dbcore::file_mgr
{
    namespace detail
    {
        std::string temp_prefix = "temp";
    }

    file_mgr::file_mgr(const std::filesystem::path& db_file_directory, int blk_size)
        : db_directory(db_file_directory), block_size(blk_size)
    {
        is_new = !std::filesystem::exists(db_directory);

        if (is_new)
            std::filesystem::create_directories(db_directory);

        for (const auto& entry : std::filesystem::directory_iterator(db_directory))
        {
            if (entry.path().filename().string().find(detail::temp_prefix) == 0)
                std::filesystem::remove(entry.path());
        }
    }

    void file_mgr::read(const block_id& blk, page& p)
    {
        auto& handler = get_file_handler(blk.get_filename());

        std::shared_lock<std::shared_mutex> lock(handler.mutex);

        try
        {
            auto& file = handler.file;
            file.seekg(blk.get_block_number() * block_size);
            file.read(p.contents().data(), block_size);

            if (!file)
            {
                if (file.eof())
                    std::fill(p.contents().begin(), p.contents().end(), 0);
                else
                    throw std::runtime_error("Failed to read entire block, file_mgr.cpp");
            }
        }
        catch(const std::exception& e)
        {
            throw std::runtime_error(fmt::format("Cannot read block {} : {}, src: {}", blk.to_string(), e.what(), "file_mgr.cpp"));
        }  
    }

    void file_mgr::write(const block_id& blk, page& p)
    {   
        auto& handler = get_file_handler(blk.get_filename());

        std::unique_lock<std::shared_mutex> lock(handler.mutex);
        
        try {
            auto& file = handler.file;
            file.seekp(blk.get_block_number() * block_size);
            file.write(p.contents().data(), block_size);
            file.flush();
            if (!file) {
                throw std::runtime_error("Failed to write entire block, file_mgr.cpp");
            }
        } catch (const std::exception& e) {
            throw std::runtime_error(fmt::format("Cannot write block {} : {}, src: {}", blk.to_string(), e.what(), "file_mgr.cpp"));
        }
    }

    std::unique_ptr<block_id> file_mgr::append(const std::string& filename)
    {
        auto& handler = get_file_handler(filename);

        std::unique_lock<std::shared_mutex> lock(handler.mutex);

        int new_blk_num = 0;
        std::vector<char> blk(block_size, 0);
        try
        {
            auto& file = handler.file;
            file.seekg(0, std::ios::end);
            new_blk_num = static_cast<int>(file.tellg() / block_size);

            file.seekp(new_blk_num * block_size);
            file.write(blk.data(), block_size);
            file.flush();

            if (!file)
                throw std::runtime_error(fmt::format("Failed to append block to file {} src: {}", filename, "file_mgr.cpp"));
        }
        catch(const std::exception& e)
        {
            throw std::runtime_error(fmt::format("Cannot append block to file {} : {}, src: {}", filename, e.what(), "file_mgr.cpp"));
        }
        return std::make_unique<block_id>(filename, new_blk_num);
    }

    int file_mgr::len(const std::string& filename)
    {
        auto& handler = get_file_handler(filename);

        std::shared_lock<std::shared_mutex> lock(handler.mutex);

        try
        {
            auto& file = handler.file;
            file.seekg(0, std::ios::end);
            return static_cast<int>(file.tellg() / block_size);
        }
        catch(const std::exception& e)
        {
            throw std::runtime_error(fmt::format("Cannot access file {} : {}, src: {}", filename, e.what(), "file_mgr.cpp"));
        }
        
    }

    bool file_mgr::is_new_db() const
    {
        return is_new;
    }

    int file_mgr::get_block_size() const
    {
        return block_size;
    }
}