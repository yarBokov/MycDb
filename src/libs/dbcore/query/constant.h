#ifndef __QUERY_CONSTANT_H
#define __QUERY_CONSTANT_H

#include <variant>
#include <string>
#include <memory>
#include <type_traits>

#include "libs/dbcore/exceptions/query_constant_variant_exception.h"

namespace dbcore::query
{
    class constant
    {
        private:
            std::variant<int, std::string> m_value;

        public:
            constant() = default;
            constant(int ival): m_value(ival) {}
            constant(const std::string& sval): m_value(sval) {}
            constant(std::string&& sval) noexcept : m_value(std::move(sval)) {}

            constant(const constant&) = default;
            constant(constant&&) noexcept = default;
            constant& operator=(const constant&) = default;
            constant& operator=(constant&&) noexcept = default;
            ~constant() = default;

            int as_int() const
            {
                if (auto* pval = std::get_if<int>(&m_value))
                    return *pval;
                throw exceptions::query_constant_variant_exception("Not an integer constant");
            }

            std::string as_str() const
            {
                if (auto* pval = std::get_if<std::string>(&m_value))
                    return *pval;
                throw exceptions::query_constant_variant_exception("Not a string constant");
            }

            bool operator== (const constant& other) const
            {
                return m_value == other.m_value;
            }

            bool operator< (const constant& other) const
            {
                return m_value < other.m_value;
            }
            
            std::string to_string() const
            {
                return std::visit([](auto&& arg) -> std::string {
                    using Type = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<Type, int>)
                        return std::to_string(arg);
                    else if constexpr (std::is_same_v<Type, std::string>)
                        return arg;
                }, m_value);
            }

            struct hash
            {
                std::size_t operator()(const constant& c)
                {
                    return std::visit([](auto&& arg) -> std::size_t {
                        using Type = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<Type, int>)
                            return std::hash<int>()(arg);
                        else if constexpr (std::is_same_v<Type, std::string>)
                            return std::hash<std::string>()(arg);
                    }, c.m_value);
                }
            };
    };
}

#endif