#ifndef __FILE_MGR_IMPL_H
#define __FILE_MGR_IMPL_H

#include <string>
#include <unordered_map>
#include <memory>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <shared_mutex>

#include "page/block_id.h"
#include "page/page.h"

#include <fmt/format.h>

namespace dbcore::file_mgr
{
    class file_mgr
    {
        private:
            std::filesystem::path db_directory;
            int block_size;
            bool is_new;

            struct file_handler
            {
                std::fstream file;
                std::shared_mutex mutex;
            };

            std::unordered_map<std::string, std::unique_ptr<file_handler>> opened_files;
            mutable std::mutex opened_files_mtx;

            inline file_handler& get_file_handler(const std::string& filename)
            {
                {
                    std::shared_lock<std::mutex> lock(opened_files_mtx);
                    auto it = opened_files.find(filename);
                    if (it != opened_files.end())
                        return *(it->second);
                }
                
                std::unique_lock<std::mutex> lock(opened_files_mtx);

                auto it = opened_files.find(filename);
                if (it != opened_files.end())
                    return *(it->second);
                
                auto db_table = db_directory / filename;
                auto handler = std::make_unique<file_handler>();

                handler->file.open(db_table, std::ios::in | std::ios::out | std::ios::binary);
                if (!handler->file.is_open())
                {
                    handler->file.open(db_table, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
                    if (!handler->file.is_open())
                        throw std::runtime_error(fmt::format("Cannot open file {}, {}", db_table.string(), "file_mgr.cpp"));
                }

                auto res = opened_files.emplace(filename, std::move(handler));
                return *(res.first->second);
            }
        
        public:
            file_mgr() = default;
            ~file_mgr() = default;

            file_mgr(const std::filesystem::path& db_file_directory, int blk_size);

            void read(const block_id& blk, page& p);
            void write(const block_id& blk, page& p);

            std::unique_ptr<block_id> append(const std::string& filename);
            int len(const std::string& filename);

            bool is_new_db() const;
            int get_block_size() const;
    };
}

#endif