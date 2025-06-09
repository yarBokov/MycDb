#ifndef __METADATA_STAT_TABLE_H
#define __METADATA_STAT_TABLE_H

#include "libs/dbcore/tx/transaction.h"
#include "libs/dbcore/record/layout.h"
#include "libs/dbcore/record/table_scan.h"
#include "constants.h"

#include <memory>

namespace dbcore::metadata
{
    class table_mgr
    {
        public:
            static const int MAX_NAME = 16;

        private:
            std::unique_ptr<record::layout> m_tcat_layout;
            std::unique_ptr<record::layout> m_fcat_layout;
            
            void create_catalog_tables(bool isNew, std::shared_ptr<tx::transaction> tx);

        public:
            table_mgr(bool is_new, std::shared_ptr<tx::transaction> tx)
            {
                using namespace record;
                using namespace detail;
                schema tcat_schema;
                tcat_schema.add_str_field(tblname_field, MAX_NAME);
                tcat_schema.add_int_field(slotsize_field);
                m_tcat_layout = std::make_unique<layout>(tcat_schema);

                schema fcat_schema;
                fcat_schema.add_str_field(tblname_field, MAX_NAME);
                fcat_schema.add_str_field(fldname_field, MAX_NAME);
                fcat_schema.add_int_field(type_field);
                fcat_schema.add_int_field(length_field);
                fcat_schema.add_int_field(offset_field);
                m_fcat_layout = std::make_unique<layout>(fcat_schema);

                if (is_new)
                {
                    create_table(table_catalog_tbl, tcat_schema, tx);
                    create_table(fields_catalog_tbl, fcat_schema, tx);
                }
            }

            void create_table(const std::string& tblname, record::schema& sch, std::shared_ptr<tx::transaction> tx)
            {
                using namespace record;
                layout layout(sch);
    
                // Insert into tblcat
                table_scan tcat(tx, detail::table_catalog_tbl, *m_tcat_layout);
                tcat.insert();
                tcat.set_str(detail::tblname_field, tblname);
                tcat.set_int(detail::slotsize_field, layout.slot_size());
                tcat.close();

                // Insert into fldcat for each field
                table_scan fcat(tx, detail::fields_catalog_tbl, *m_fcat_layout);
                for (const auto& fldname : sch.fields()) {
                    fcat.insert();
                    fcat.set_str(detail::tblname_field, tblname);
                    fcat.set_str(detail::fldname_field, fldname);
                    fcat.set_int(detail::type_field, static_cast<int>(sch.type(fldname)));
                    fcat.set_int(detail::length_field, sch.length(fldname));
                    fcat.set_int(detail::offset_field, layout.offset(fldname));
                }
                fcat.close();
            }

            std::unique_ptr<record::layout> get_layout(const std::string& tblname, std::shared_ptr<tx::transaction> tx)
            {
                using namespace record;
                using namespace detail;

                int size = -1;
                table_scan tcat(tx, table_catalog_tbl, *m_tcat_layout);
                while (tcat.next()) {
                    if (tcat.get_str(tblname_field) == tblname) {
                        size = tcat.get_int(slotsize_field);
                        break;
                    }
                }
                tcat.close();

                schema sch;
                std::unordered_map<std::string, int> offsets;
                table_scan fcat(tx, fields_catalog_tbl, *m_fcat_layout);
                while (fcat.next()) 
                {
                    if (fcat.get_str(tblname_field) == tblname) {
                        std::string fldname = fcat.get_str(fldname_field);
                        int fldtype = fcat.get_int(type_field);
                        int fldlen = fcat.get_int(length_field);
                        int offset = fcat.get_int(offset_field);
                        offsets[fldname] = offset;
                        sch.add_field(fldname, static_cast<schema::sql_types>(fldtype), fldlen);
                    }
                }
                fcat.close();
                
                return std::make_unique<layout>(sch, offsets, size);
            }
    };
}

#endif