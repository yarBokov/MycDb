#include "page.h"
#include <stdexcept>
#include <algorithm>

#include "libs/dbcore/detail/check_int_alignment.h"
#include <fmt/format.h>

namespace dbcore::file_mgr
{
    const std::string page::CHARSET = "CP866"; // russian

    page::page(int block_size) : buffer(block_size) {}

    page::page(const std::vector<char>& buf) : buffer(buf) {}

    int page::get_int(int offset) const
    {
        check_alignment(offset);
        if (offset + sizeof(int) > buffer.size())
            throw std::out_of_range("Invalid offset for get_int, src: page.cpp");
        int value = 0;
        std::memcpy(&value, buffer.data() + offset, sizeof(int));
        return value;
    }

    std::vector<char> page::get_bytes(int offset) const
    {
        if (offset + sizeof(int) > buffer.size())
            throw std::out_of_range("Invalid offset for get_bytes, src: page.cpp");

        int len = this->get_int(offset);
        if (offset + len + sizeof(int) > buffer.size())
            throw std::out_of_range("Invalid byte array length, src: page.cpp");
        
        return std::vector<char>(buffer.begin()+ offset + sizeof(int),
                                buffer.begin() + offset + sizeof(int) + len);
    }

    std::string page::get_string(int offset) const
    {
        auto bytes = this->get_bytes(offset);
        return std::string(bytes.begin(), bytes.end());
    }

    void page::set_int(int offset, int num)
    {
        check_alignment(offset);
        if (offset + sizeof(int) > buffer.size()) 
            throw std::out_of_range("Invalid offset for set_int, src: page.cpp");

        std::memcpy(buffer.data() + offset, &num, sizeof(int));
    }

    void page::set_bytes(int offset, const std::vector<char>& buf)
    {
        if (offset + sizeof(int) + buf.size() > buffer.size())
            throw std::out_of_range("Not enough space for set_bytes operation, src: page.cpp");

        this->set_int(offset, static_cast<int>(buf.size()));
        std::copy(buf.begin(), buf.end(), buffer.begin() + offset + sizeof(int));
    }

    void page::set_string(int offset, const std::string& str)
    {
        std::vector<char> bytes(str.begin(), str.end());
        this->set_bytes(offset, bytes);
    }

    int page::max_len(int strlen)
    {
        return sizeof(int) + strlen;
    }

    const std::vector<char>& page::contents() const
    {
        return buffer;
    }

    std::vector<char>& page::contents()
    {
        return buffer;
    }

    void page::check_alignment(int offset) const
    {
        if (!dbcore::check_int_alignment(offset))
            throw std::runtime_error(fmt::format("Unaligned access at offset {}, src: {}", std::to_string(offset), "page.cpp"));
    }
}