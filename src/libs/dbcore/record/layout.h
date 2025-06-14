#ifndef __RECORD_LAYOUT_H
#define __RECORD_LAYOUT_H

#include "libs/dbcore/file/page/page.h"
#include "schema.h"

#include <unordered_map>
#include <memory>

namespace dbcore::record
{
    class layout
    {
        private:
            std::shared_ptr<schema> m_schema;
            std::unordered_map<std::string, int> m_offsets;
            int m_slotsize;

            int length_in_bytes(const std::string& fldname) const
            {
                schema::sql_types fldtype = m_schema->type(fldname);
                if (fldtype == schema::sql_types::integer)
                    return sizeof(int);
                else return file_mgr::page::max_len(m_schema->length(fldname));
            }

        public:
            layout() = default;
            layout(std::shared_ptr<schema> schema)
                : m_schema(schema)
            {
                int pos = sizeof(int);
                for (const auto& fldname : schema->fields())
                {
                    m_offsets[fldname] = pos;
                    pos += length_in_bytes(fldname);
                }
            }

            layout(std::shared_ptr<schema> schema, 
                const std::unordered_map<std::string, int>& offsets, 
                int slotsize)
                : m_schema(schema), m_offsets(offsets), m_slotsize(slotsize)
            {}

            layout& operator=(const layout& other)
            {
                m_schema = other.m_schema;
                m_slotsize = other.m_slotsize;
                m_offsets = other.m_offsets;
                return *this;
            }
            
            std::shared_ptr<schema> get_schema() const { return m_schema; }
            int offset(const std::string& fldname) const { return m_offsets.at(fldname); }
            int slot_size() const { return m_slotsize; }
    };
}

#endif