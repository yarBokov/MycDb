#ifndef __RECORD_SCHEMA_H
#define __RECORD_SCHEMA_H

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace dbcore::record
{
    class schema
    {
        public:
            enum class sql_types
            {
                integer = 4,
                varchar = 12
            };
    
        private:
            struct field_info;

            std::vector<std::string> m_fields;
            std::unordered_map<std::string, std::unique_ptr<field_info>> m_info;

            struct field_info {
                sql_types type;
                int length;
                
                field_info(sql_types type, int length) : type(type), length(length) {}
            };

        public:
            schema() = default;

            schema(const schema& other)
            {
                for (const auto& fldname : other.m_fields) {
                    auto& info = other.m_info.at(fldname);
                    this->add_field(fldname, info->type, info->length);
                }
            }

            schema& operator=(const schema& other)
            {
                if (this != &other) {
                    m_fields.clear();
                    m_info.clear();
                    for (const auto& fldname : other.m_fields) {
                        auto& info = other.m_info.at(fldname);
                        this->add_field(fldname, info->type, info->length);
                    }
                }
                return *this;
            }
            
            void add_field(const std::string& fldname, sql_types type, int length)
            {
                m_fields.push_back(fldname);
                m_info[fldname] = std::make_unique<field_info>(type, length);
            }

            void add_int_field(const std::string& fldname)
            {
                add_field(fldname, sql_types::integer, 0);
            }

            void add_str_field(const std::string& fldname, int length)
            {
                add_field(fldname, sql_types::varchar, length);
            }
            
            void add(const std::string& fldname, const schema& sch)
            {
                sql_types type = sch.type(fldname);
                int length = sch.length(fldname);
                add_field(fldname, type, length);
            }

            void add(const std::string& fldname, std::shared_ptr<schema> sch)
            {
                sql_types type = sch->type(fldname);
                int length = sch->length(fldname);
                add_field(fldname, type, length);
            }

            void add_all(const schema& sch)
            {
                for (const std::string& fldname : sch.fields()) 
                {
                    add(fldname, sch);
                }
            }

            void add_all(std::shared_ptr<schema> sch)
            {
                for (const std::string& fldname : sch->fields()) 
                {
                    add(fldname, sch);
                }
            }
            
            const std::vector<std::string>& fields() const
            {
                return m_fields;
            }

            bool has_field(const std::string& fldname) const
            {
                return m_info.find(fldname) != m_info.end();
            }

            sql_types type(const std::string& fldname) const
            {
                return m_info.at(fldname)->type;
            }

            int length(const std::string& fldname) const
            {
                return m_info.at(fldname)->length;
            }
        };
}

#endif