#ifndef __ENCRYPT_FILE_MGR_IMPL_H
#define __ENCRYPT_FILE_MGR_IMPL_H

#include "file_mgr.h"
#include "page/page_crypt.h"

namespace engine::file_mgr
{
    class secure_file_mgr : public file_mgr
    {
        private:
            page_encryptor encryptor;
            page temp_page;
        public:
            secure_file_mgr() = delete;
            secure_file_mgr(int block_size, const std::vector<unsigned char>& encryption_key)
                : file_mgr(block_size)
                , encryptor(block_size, encryption_key)
                , temp_page(block_size)
            {}

            ~secure_file_mgr() = default;

            void read(const block_id& blk, page& p) override
            {
                file_mgr::read(blk, temp_page);
                encryptor.decrypt(temp_page, page);
            }

            void write(const block_id& blk, page& p) override
            {
                encryptor.encrypt(p, temp_page);
                file_mgr::write(blk, temp_page);
            }
    };
}

#endif