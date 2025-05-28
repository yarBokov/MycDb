#ifndef __FILE_MGR_PAGE_H
#define __FILE_MGR_PAGE_H

#include <vector>
#include <string>
#include <cstring>

namespace dbcore::file_mgr
{
    class page
    {
        private:
            std::vector<char> buffer;
            static const std::string CHARSET;

            void check_alignment(int offset) const;

        public:
            page() = default;
            explicit page(int block_size);

            explicit page(const std::vector<char>& buf);

            ~page() = default;

            int get_int(int offset) const;
            void set_int(int offset, int num);

            std::vector<char> get_bytes(int offset) const;
            void set_bytes(int offeset, const std::vector<char>& buf);

            std::string get_string(int offset) const;
            void set_string(int offset, const std::string& str);

            static int max_len(int strlen);

            [[nodiscard]] const std::vector<char>& contents() const;
            [[nodiscard]] std::vector<char>& contents();
    };
}

#endif